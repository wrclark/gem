#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "request.h"
#include "response.h"
#include "file.h"

struct gem_config cfg;

static void usage(char *argv[]);
static void startup_check(void);

int main(int argc, char *argv[]) {
    SSL_CTX *ctx;
    SSL *ssl;
    struct sockaddr_in addr;
    struct timeval timeout;
    int fd, client;
    int enable = 1;
    int opt, err;
    int hset = 0, dset = 0;
    int pset = 0, iset = 0;
    int kset = 0, cset = 0;

    struct gem_uri uri = {0};
    char buffer[GEM_URI_MAXSIZ + 1] = {0};

    /* Force line-buffering */
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    while ((opt = getopt(argc, argv, "k:c:h:p:d:i:eav")) != -1) {
        switch (opt) {
            case 'k':
                strncpy(cfg.key_path, optarg, GEM_CFG_SIZ);
                kset = 1;
                break;
            case 'c':
                strncpy(cfg.crt_path, optarg, GEM_CFG_SIZ);
                cset = 1;
                break;
            case 'h':
                strncpy(cfg.hostname, optarg, GEM_CFG_SIZ);
                hset = 1;
                break;
            case 'p':
                cfg.port = atoi(optarg);
                pset = 1;
                break;
            case 'd':
                strncpy(cfg.docroot, optarg, GEM_CFG_SIZ);
                dset = 1;
                break;
            case 'i':
                strncpy(cfg.index, optarg, GEM_CFG_SIZ);
                iset = 1;
                break;
            case 'e':
                cfg.enumerate = 1;
                break;
            case 'a':
                cfg.diffhost = 1;
                break;
            case 'v':
                cfg.verbose = 1;
                break;
            default:
                usage(argv);
        }
    }

    /* check user input */

    /* port provided as 0 or atoi() failed */
    if (!cfg.port) {
        if (pset) {
            fprintf(stderr, "invalid port\n");
            usage(argv);
        }

        cfg.port = 1965;
    }

    /* doc root not set */
    if (!dset) {
        fprintf(stderr, "Error: no document root path specified\n");
        usage(argv);
    }

    /* hostname not set */
    if (!hset) {
        strncpy(cfg.hostname, "localhost", GEM_CFG_SIZ);
    }

    /* index file not set */
    if (!iset) {
        strncpy(cfg.index, "index.gmi", GEM_CFG_SIZ);
    }

    if (cfg_validate(&cfg)) {
        usage(argv);
    }

    /* If not explicitly set when calling program, use defaults.. */
    if (!kset) {
        strcpy(cfg.key_path, PRIVATE_KEY_PATH);
    }

    if (!cset) {
        strcpy(cfg.crt_path, PUBLIC_KEY_PATH);
    }

    /* check ssl/tls files */
    startup_check();

    /* timeout CLIENT sockets after 10 seconds */
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket()");
        exit(1);
    }

    /* useful for quick restarts */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof enable) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(fd);
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)cfg.port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind()");
        close(fd);
        exit(1);
    }

    /* prevent zombie processes */
    signal(SIGCHLD, SIG_IGN);

    while(1) {
        listen(fd, 10);

        client = accept(fd, NULL, NULL);

        /* timeout on receive */
        if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
            perror("setsockopt(SO_RCVTIMEO)");
            close(client);
            continue;
        }

        /* timeout on send */
        if (setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0) {
            perror("setsockopt(SO_SNDTIMEO)");
            close(client);
            continue;
        }

        if (!fork()) {

            memset(buffer, 0, GEM_URI_MAXSIZ);
            memset(&uri, 0, sizeof (uri));

            ctx = SSL_CTX_new(TLS_server_method());
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client);

            SSL_use_certificate_chain_file(ssl, cfg.crt_path);
            SSL_use_PrivateKey_file(ssl, cfg.key_path, SSL_FILETYPE_PEM);

            if (SSL_accept(ssl) != 1) {
                goto CLOSE_CONNECTION;
            }
            
            if (SSL_read(ssl, buffer, GEM_URI_MAXSIZ) <= 0) {
                goto CLOSE_CONNECTION;
            }

            /* chroot "/" to docroot */
            if (chroot(cfg.docroot)) {
                perror("unable to chroot docroot");
                goto CLOSE_CONNECTION;
            }

            if (chdir("/") != 0) {
                goto CLOSE_CONNECTION;
            }

            if (cfg.verbose) printf("\ndata received: %s\n", buffer);
            
            request_parse(buffer, &uri);
            if (cfg.verbose) request_print_uri(&uri);

            if (uri.error != 0) {
                if (cfg.verbose) printf("(1) E=%d\n", uri.error);
                resp_error(RESP_STATUS_BAD_REQUEST, ssl);
                goto CLOSE_CONNECTION;
            }

            request_validate_uri(&uri);

            if (uri.error != 0) {
                if (cfg.verbose) printf("(2) E=%d\n", uri.error);
                switch (uri.error) {
                    case REQUEST_ERR_WRONG_SCHEME:
                    case REQUEST_ERR_WRONG_DOMAIN:
                    case REQUEST_ERR_PORT:
                        resp_error(RESP_STATUS_PROXY_REFUSED, ssl);
                    break;
                    case REQUEST_ERR_PATH:
                    case REQUEST_ERR_SCHEME:
                    case REQUEST_ERR_DOMAIN:
                        resp_error(RESP_STATUS_BAD_REQUEST, ssl);
                    break;                  
                }
                goto CLOSE_CONNECTION;
            }

            if ((err = resp_serve_file(&uri, ssl))) {
                switch(err) {
                    case RESP_FILE_NOT_FOUND:
                        if (cfg.verbose) puts("file not found");
                        resp_error(RESP_STATUS_NOT_FOUND, ssl);
                    break;
                    case RESP_FILE_TRANSFER:
                        if (cfg.verbose) puts("file transfer failed");
                    break;
                    case RESP_FILE_PATH_TOO_LONG:
                        if (cfg.verbose) puts("provided file path too long");
                        break;
                    default:
                        if (cfg.verbose) printf("unknown error: %d\n", err);
                }
                goto CLOSE_CONNECTION;
            }

            if (cfg.verbose) puts("OK");

CLOSE_CONNECTION:
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(client);
            _exit(0);
        }
    }

    exit(0);

}

/* print usage and exit */
static void usage(char *argv[]) {
    fprintf(stderr, "Usage:\n%s [OPTIONS]\n", argv[0]);
    fprintf(stderr, "\t-c [pub cert path]   ex: -c \"gem.crt\"     (tls/server.crt default)\n");
    fprintf(stderr, "\t-k [priv key path]   ex: -k \"gem.key\"     (tls/server.key default)\n");
    fprintf(stderr, "\t-h [HOSTNAME]        ex: -h \"example.com\" (localhost default)\n");
    fprintf(stderr, "\t-p [PORT]            ex: -p 1965          (default)\n");
    fprintf(stderr, "\t-d [DOC ROOT]        ex: -d \"/var/gemini\"\n");
    fprintf(stderr, "\t-i [INDEX FILE]      ex: -i \"index.gmi\"   (default)\n");
    fprintf(stderr, "\t-e  enumerate directories without an index file\n");
    fprintf(stderr, "\t-a  permit requests with a different hostname\n");
    fprintf(stderr, "\t-v  print request information\n");

    exit(1);
}

static void startup_check(void) {

    if (!file_exists(cfg.crt_path) || !file_exists(cfg.key_path)) {
        fprintf(stderr, "unable to find ssl keys\n");
        fprintf(stderr, "Public key: %s\n", cfg.crt_path);
        fprintf(stderr, "Private key: %s\n", cfg.key_path);
        exit (1);
    }
}


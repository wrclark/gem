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

struct gem_config cfg;

/* print usage and exit */
static void usage(int argc, char *argv[]) {
    fprintf(stderr, "Usage:\n%s [OPTIONS]\n", argv[0]);
    fprintf(stderr, "\t-h [HOSTNAME]   ex: -h \"example.com\"   (localhost default)\n");
    fprintf(stderr, "\t-p [PORT]       ex: -p 1965            (default)\n");
    fprintf(stderr, "\t-d [DOC ROOT]   ex: -d \"/var/gemini\"\n");
    fprintf(stderr, "\t-i [INDEX FILE] ex: -i \"index.gmi\"     (default)\n");
    fprintf(stderr, "\t-e  enumerate directories without an index file\n");
    fprintf(stderr, "\t-a  permit requests with a different hostname\n");
    (void) argc;

    exit(1);
}

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

    struct gem_uri uri = {0};
    char buffer[GEM_URI_MAXSIZ + 1] = {0};

    while ((opt = getopt(argc, argv, "h:p:d:i:ea")) != -1) {
        switch (opt) {
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
            default:
                usage(argc, argv);
        }
    }

    /* check user input */

    /* port provided as 0 or atoi() failed */
    if (!cfg.port) {
        if (pset) {
            fprintf(stderr, "invalid port\n");
            usage(argc, argv);
        }

        cfg.port = 1965;
    }

    /* doc root not set */
    if (!dset) {
        fprintf(stderr, "Error: no document root path specified\n");
        usage(argc, argv);
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
        usage(argc, argv);
    }

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

            SSL_use_certificate_chain_file(ssl, PUBLIC_KEY);
            SSL_use_PrivateKey_file(ssl, PRIVATE_KEY, SSL_FILETYPE_PEM);

            if (SSL_accept(ssl) != 1) {
                goto CLOSE_CONNECTION;
            }
            
            if (SSL_read(ssl, buffer, GEM_URI_MAXSIZ) <= 0) {
                goto CLOSE_CONNECTION;
            }

            printf("\ndata received: %s\n", buffer);
            
            request_parse(buffer, &uri);
            request_print_uri(&uri);

            if (uri.error != 0) {
                printf("(1) E=%d\n", uri.error);
                resp_error(RESP_STATUS_BAD_REQUEST, ssl);
                goto CLOSE_CONNECTION;
            }

            request_validate_uri(&uri);

            if (uri.error != 0) {
                printf("(2) E=%d\n", uri.error);
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
                        puts("file not found");
                        resp_error(RESP_STATUS_NOT_FOUND, ssl);
                    break;
                    case RESP_FILE_TRANSFER:
                        puts("file transfer failed");
                    break;
                    default:
                        printf("unknown error: %d\n", err);
                }
                goto CLOSE_CONNECTION;
            }

            puts("OK");

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

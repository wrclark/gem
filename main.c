#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "config.h"
#include "request.h"
#include "response.h"

int main(int argc, char *argv[]) {
    SSL_CTX *ctx;
    SSL *ssl;
    struct sockaddr_in addr;
    int fd, client;
    int opt = 1;
    int err;

    struct gem_uri uri = {0};
    char buffer[GEM_URI_MAXSIZ + 1] = {0};

    (void) argc;
    (void) argv;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket()");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt)) {
        close(fd);
        perror("setsockopt()");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(GEM_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind()");
        exit(1);
    }

    while(1) {
        listen(fd, 10);

        client = accept(fd, NULL, NULL);

        if (!fork()) {
            memset(buffer, 0, GEM_URI_MAXSIZ);
            memset(&uri, 0, sizeof (uri));

            ctx = SSL_CTX_new(TLS_server_method());
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client);

            SSL_use_certificate_chain_file(ssl, PUBLIC_KEY);
            SSL_use_PrivateKey_file(ssl, PRIVATE_KEY, SSL_FILETYPE_PEM);

            SSL_accept(ssl);
            SSL_read(ssl, buffer, GEM_URI_MAXSIZ);

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
        }
    }

    exit(0);

}

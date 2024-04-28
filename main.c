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
    int err;
    struct request *req;
    struct resource *res;
    int opt = 1;

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

            req = calloc(1, sizeof (struct request));
            res = calloc(1, sizeof (struct resource));

            ctx = SSL_CTX_new(TLS_server_method());
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client);

            SSL_use_certificate_chain_file(ssl, PUBLIC_KEY);
            SSL_use_PrivateKey_file(ssl, PRIVATE_KEY, SSL_FILETYPE_PEM);

            SSL_accept(ssl);
            req->size = SSL_read(ssl, &req->data, 1024);

            printf("data received: %s\n", req->data);

            /* if the request is 'invalid' .. */
            if ((err = req_valid(req))) {
                if (err == 10) {
                    resp_error(RESP_STATUS_PROXY_REFUSED, ssl);
                } else {
                    resp_error(RESP_STATUS_BAD_REQUEST, ssl);
                }
                printf("invalid request! E=%d\n", err);
                goto CLOSE_CONNECTION;
            }

            puts("request OK");

            /* if the resource requested is bad */
            if ((err = req_resource(req, res))) {
                resp_error(RESP_STATUS_PERMFAIL, ssl);
                printf("invalid resource! E=%d\n", err);
                goto CLOSE_CONNECTION;
            }

            puts("resource OK");

            /* URIs ending in / should redirect to /index.gmi */
            /* eg gemini://example.com/cats/ -> /cats/index.gmi */
            /* URI of "gemini://example.com" -> /index.gmi */
            if (req_check_index(res)) {
                resp_redirect("gemini://"GEM_HOSTNAME"/", ssl);
                puts("redirected \"\" -> \"/\"");
            }

            puts("index OK");

            /* file does not exist */
            if (resp_file_exists(res)) {
                resp_error(RESP_STATUS_NOT_FOUND, ssl);
                printf("file does not exist: %s\n", res->data);
                goto CLOSE_CONNECTION;
            }

            printf("file exists !! : %s\n", res->data);

            /* file transfer failed */
            if (resp_file_transfer(res, ssl)) {
                puts("file transfer failed");
                goto CLOSE_CONNECTION;
            }

            printf("resource requested: %s\n", res->data);

CLOSE_CONNECTION:
            free(req);
            free(res);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
        }
    }

    exit(0);

}

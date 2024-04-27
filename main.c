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

const char *response = "20 text/gemini\r\n#Hello\n*Good morning\n*Sir\n";
const char *tempfail = "40 text/plain\r\nBad request\n";
const char *permfail = "50 text/plain\r\nFile not found\n";


int main(int argc, char *argv[]) {
    SSL_CTX *ctx;
    SSL *ssl;
    struct sockaddr_in addr;
    int fd, client;
    int err;
    struct request *req;
    struct resource *res;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket()");
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
                SSL_write(ssl, tempfail, strlen(tempfail));
                printf("invalid request! E=%d\n", err);
                goto CLOSE_CONNECTION;
            }

            /* if the resource requested is bad */
            if ((err = req_resource(req, res))) {
                SSL_write(ssl, tempfail, strlen(tempfail));
                printf("invalid resource! E=%d\n", err);
                goto CLOSE_CONNECTION;
            }

            /* 1. check if resource exists 
             * 2. if no, send gemini 404 and die
             * 3.1 send 20 and mime type determimnned by the the extension
             * 3.2 start sending the file
             * 3.3 die
             */

            /* file does not exist */
            if (resp_file_exists(res)) {
                SSL_write(ssl, permfail, strlen(permfail));
                printf("file does not exist: %s\n", res->data);
                goto CLOSE_CONNECTION;
            }

            /* file transfer failed */
            if (resp_file_transfer(res, ssl)) {
                SSL_write(ssl, tempfail, strlen(tempfail));
                goto CLOSE_CONNECTION;
            }

            printf("resource requested: %s\n", res->data);
            SSL_write(ssl, response, strlen(response));

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

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "request.h"

const char *hostname = "localhost";
const char *docroot = "capsule/";

#define PUBLIC_KEY "tls/server.crt"
#define PRIVATE_KEY "tls/server.key"

int main(int argc, char *argv[]) {
    int fd, client;
    struct sockaddr_in addr;
    SSL_CTX *ctx;
    SSL *ssl;
    char buffer[1024] = {0};
    char *response = "20 text/gemini\r\n#Hello!!!!";

    fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1965);
    inet_aton("0.0.0.0", &addr.sin_addr.s_addr);

    bind(fd, (struct sockaddr *)&addr, sizeof addr);

    while(1) {
        listen(fd, 10);

        client = accept(fd, NULL, NULL);

        if (!fork()) {
            ctx = SSL_CTX_new(TLS_server_method());
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client);

            SSL_use_certificate_chain_file(ssl, PUBLIC_KEY);
            SSL_use_PrivateKey_file(ssl, PRIVATE_KEY, SSL_FILETYPE_PEM);

            SSL_accept(ssl);
            SSL_read(ssl, buffer, 1023);

            printf("data received: %s\n", buffer);

            SSL_write(ssl, response, strlen(response));
            SSL_shutdown(ssl);
        }
    }

    exit(0);

}

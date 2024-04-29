#include <stdio.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

#include "response.h"
#include "mime.h"
#include "config.h"

/* returns 1 if the file exists */
static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* attempt to transfer the file over ssl */
/* non-zero return means error */
/* TODO chunks */
static int file_transfer(const char *path, SSL *ssl) {
    char *mime;
    char *buf;
    int err;
    FILE *f;
    size_t n;

    err = 1;

    if (!(mime = mime_type(path))) {
        puts("mime error");
        return 1;
    }

    if (!(f = fopen(path, "r"))) {
        puts("fopen() error");
        return 2;
    }

    SSL_write(ssl, "20 ", strlen("20 "));
    SSL_write(ssl, mime, strlen(mime));
    SSL_write(ssl, "\r\n", strlen("\r\n"));

    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (!(buf = calloc(1, n + 1)))
        goto EXIT;
    
    if (!fread(buf, n, 1, f)) {
        printf("fread() error\n");
    }

    SSL_write(ssl, buf, n);

    err = 0;
    free(buf);

EXIT:
    fclose(f);
    return err;
}

/* attempts to locate a file in the docroot and serve it */
/* non-zero return means an error has occurred */
/* TODO: if path is a dir then list its contents unless an index.gmi exists within it */
int resp_serve_file(struct gem_uri *u, SSL *ssl) {
    char buf[2048];
    sprintf(buf, "%s%s", GEM_DOCROOT, u->path);

    if (!file_exists(buf)) {
        return RESP_FILE_NOT_FOUND;
    }

    if (file_transfer(buf, ssl)) {
        return RESP_FILE_TRANSFER;
    }

    return 0;
}

/* redirect to PATH for whatever reason */
int resp_redirect(const char *path, SSL *ssl) {
    SSL_write(ssl, "30 ", 3);
    SSL_write(ssl, path, strlen(path));
    return 0;
}

int resp_error(const char *code, SSL *ssl) {
    SSL_write(ssl, code, strlen(code));
    SSL_write(ssl, "\r\n", 2);
    return 0;
}

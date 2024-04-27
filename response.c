#include <openssl/ssl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "response.h"
#include "mime.h"
#include "config.h"

/* determine if the file exists within the docroot */
int resp_file_exists(struct resource *res) {
    char buf[2048];
    struct stat st;

    if (!res) {
        return 1;
    }

    strcpy(buf, GEM_DOCROOT);
    strncpy(buf + strlen(GEM_DOCROOT),  res->data, 2048 - strlen(GEM_DOCROOT));
    
    printf("file exists=\"%s\"\n", buf);

    /* returns 0 if the file exists */
    return stat(buf, &st);
}

/* attempt to (in chunks*) transfer the requested file */
int resp_file_transfer(struct resource *res, SSL *ssl) {
    char *mime;
    FILE *f;
    char buffer[2048] = {0};
    char *buf;
    int n, err;

    err = 1;

    /* error */
    mime = mime_type(res);
    if (!mime) {
        puts("mime error");
        return 1;
    }

    sprintf(buffer, "%s%s", GEM_DOCROOT, res->data);

    f = fopen(buffer, "r");
    if (!f) {
        puts("fopen() error");
        return 2;
    }

    sprintf(buffer, "20 %s\r\n%c", mime, '\0');
    SSL_write(ssl, buffer, strlen(buffer));
    memset(buffer, 0, 1024);

    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = calloc(1, n + 1);
    if (!buf)
        goto EXIT;
    
    fread(buf, n, 1, f);
    SSL_write(ssl, buf, n);

    err = 0;
    free(buf);

EXIT:
    fclose(f);
    return err;
}

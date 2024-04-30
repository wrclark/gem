#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

#include "response.h"
#include "mime.h"
#include "config.h"

/* check if a file is a directory */
/* non-zero return means that it is */
static int file_is_dir(const char *path) {
    struct stat st;

    if (!path) {
        return 0;
    }

    /* it should exist .. */
    if (stat(path, &st) != 0) {
        return 0;
    }

    return (st.st_mode & S_IFMT) == S_IFDIR;
}

/* if a path (dir) contains a certain file or not */
/* non-zero return means it does */
static int dir_has_index(const char *path) {
    char buf[2048] = {0};
    struct stat st;

    if (!path) {
        return 0;
    }

    strcpy(buf, path);

    /* check if the path ends in a / or not */
    if (buf[strlen(buf)] != '/') {
        strcpy(buf + strlen(buf), "/");
    }

    strcpy(buf + strlen(buf), GEM_INDEX_FILE);
    return stat(buf, &st) == 0;
}

/* iterate (alphanumerically) all of the files but "." and ".." */
/* write them as a gemtext document, where each file is written */
/* as a link, making distinction between other dir's and regular files. */
static void iterate_dir(const char *path, SSL *ssl) {
    char buffer[4096];
    char new_path[512];
    struct dirent **files;
    int qty;
    int is_dir;

    if (!path || !ssl) {
        return;
    }

    qty = scandir(path, &files, NULL, alphasort);
    if (qty == -1) {
        printf("scandir() error: %s\n", path);
        return;
    }

    /* remove docroot */
    strcpy(new_path, path + strlen(GEM_DOCROOT));

    SSL_write(ssl, "20 text/gemini\r\n", strlen("20 text/gemini\r\n"));
    SSL_write(ssl, "# Directory listing\n", strlen("# Directory listing\n"));
    SSL_write(ssl, "Auto-generated\n", strlen("Auto-generated\n"));

    while (qty--) {

        if (!strcmp(files[qty]->d_name, ".") || !strcmp(files[qty]->d_name, "..")) {
            continue;
        }

        is_dir = files[qty]->d_type == DT_DIR;
        memset(buffer, 0, 4096);
        sprintf(buffer, "=> gemini://%s%s%s%c   <%s> %s%c\n",
            GEM_HOSTNAME,
            new_path,
            files[qty]->d_name,
            is_dir ? '/' : ' ',
            is_dir ? "DIR" : "FILE",
            files[qty]->d_name,
            is_dir ? '/' : ' ');
        SSL_write(ssl, buffer, strlen(buffer));
    }
}

/* returns 1 if the file exists */
static int file_exists(const char *path) {
    struct stat st;

    if (!path) {
        return 0;
    }

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

    if (!path || !ssl) {
        return 3;
    }

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
    int index;
    char buf[2048];

    if (!u || !ssl) {
        return 3;
    }

    sprintf(buf, "%s%s", GEM_DOCROOT, u->path);

    /* if file does not exist */
    if (!file_exists(buf)) {
        return RESP_FILE_NOT_FOUND;
    }

    /* if file is a directory */
    if (file_is_dir(buf)) {

        /* make sure a dir path ends with / */
        if (buf[strlen(buf)] != '/') {
            strcpy(buf + strlen(buf), "/");
        }

        index = dir_has_index(buf);

        /* no index file present, and dir enumeration is enabled */
        if (!index && GEM_LIST_DIR) {
            iterate_dir(buf, ssl);
            return 0;
        }

        if (index) {
            if (u->path[strlen(u->path)] != '/') {
                strcpy(u->path + strlen(u->path), "/");
            }
            sprintf(buf, "gemini://" GEM_HOSTNAME "%s" GEM_INDEX_FILE, u->path);
            resp_redirect(buf, ssl);
            return 0;
        }

        /* no dir enum and no file to view, so send error 61 why not */
        resp_error("61", ssl);
        return 0;
    }

    /* if file transfer failed */
    if (file_transfer(buf, ssl)) {
        return RESP_FILE_TRANSFER;
    }

    return 0;
}

/* redirect to PATH for whatever reason */
int resp_redirect(const char *path, SSL *ssl) {
    SSL_write(ssl, "30 ", 3);
    SSL_write(ssl, path, strlen(path));
    SSL_write(ssl, "\r\n", 3);
    return 0;
}

int resp_error(const char *code, SSL *ssl) {
    SSL_write(ssl, code, strlen(code));
    SSL_write(ssl, "\r\n", 2);
    return 0;
}

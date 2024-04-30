#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

#include "response.h"
#include "mime.h"
#include "config.h"


/* return file size */
static size_t filesize(const char *path) {
    struct stat st;

    if (!path) {
        return 0;
    }

    if (stat(path, &st) != 0) {
        return 0;
    }

    return st.st_size;
}

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
    char file_path[4096];
    char new_path[512];
    struct dirent **files;
    int qty;
    int is_dir;
    size_t size;

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

        memset(buffer, 0, 4096);
        sprintf(file_path, "%s%s", path, files[qty]->d_name);
        is_dir = files[qty]->d_type == DT_DIR;
        
        if (is_dir) {
            sprintf(buffer, "=> gemini://" GEM_HOSTNAME "%s%s/   <DIR> %s/\n",
                    new_path, files[qty]->d_name, files[qty]->d_name);
        } else {
            size = filesize(file_path);
            sprintf(buffer, "=> gemini://" GEM_HOSTNAME "%s%s   <FILE> %s <%ld B>\n",
                    new_path, files[qty]->d_name, files[qty]->d_name, size);
        }
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

        /* if it's a directory, and the path does not end in / */
        /* then re-direct to the real url */
        if (strcmp(u->path, "/") && u->path[strlen(u->path) - 1] != '/') {
            strcpy(u->path + strlen(u->path), "/");
            printf("path=%s\n", u->path);
            resp_redirect(u->path, ssl);
            return 0;
        }

        index = dir_has_index(buf);

        /* no index file present, and dir enumeration is enabled */
        if (!index && GEM_LIST_DIR) {
            iterate_dir(buf, ssl);
            return 0;
        }

        if (index) {
            sprintf(buf, "%s" GEM_INDEX_FILE, u->path);
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

/* re-direct to a relative path */
int resp_redirect(const char *path, SSL *ssl) {
    char buf[4096];
    sprintf(buf, "30 gemini://" GEM_HOSTNAME "%s\r\n", path);
    SSL_write(ssl, buf, strlen(buf));
    return 0;
}

int resp_error(const char *code, SSL *ssl) {
    SSL_write(ssl, code, strlen(code));
    SSL_write(ssl, "\r\n", 2);
    return 0;
}

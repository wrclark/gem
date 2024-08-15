#define _DEFAULT_SOURCE
#include <dirent.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "file.h"
#include "mime.h"
#include "response.h"

extern struct gem_config cfg;

/* helper */
static int write_ssl(SSL *ssl, const char *str) {
    return SSL_write(ssl, str, (int)strlen(str));
}

/* iterate (alphanumerically) all of the files but "." and ".." */
/* write them as a gemtext document, where each file is written */
/* as a link, making distinction between other dir's and regular files. */
static void iterate_dir(const char *path, SSL *ssl) {
    char buffer[4096];
    char new_path[512];
    struct dirent **files;
    struct pfs_data pfs;
    int qty, is_dir, i;
    size_t size;

    if (!path || !ssl) {
        return;
    }

    /* get a LL of all files in path, sorted alphabetically */
    qty = scandir(path, &files, NULL, alphasort);
    if (qty == -1) {
        printf("scandir() error: %s\n", path);
        return;
    }

    /* remove docroot */
    strcpy(new_path, path + strlen(cfg.docroot));

    if (write_ssl(ssl, "20 text/gemini\r\nIndex\n") <= 0) {
        goto EXIT;
    }

    while (qty--) {

        /* skip rel files . and .. */
        if (!strcmp(files[qty]->d_name, ".") || !strcmp(files[qty]->d_name, "..")) {
            continue;
        }

        is_dir = (files[qty]->d_type == DT_DIR);
        
        if (!is_dir) {
            sprintf(buffer, "%s%s", path, files[qty]->d_name);
            size = filesize(buffer);
            pfs = pretty_filesize(size);
            if (pfs.type) {
                sprintf(buffer, "=> %s%s   <FILE> %s <%.2f %s>\n",
                    new_path, files[qty]->d_name, files[qty]->d_name, pfs.value, pfs.type);
            } else {
                sprintf(buffer, "=> %s%s   <FILE> %s <%.0f B>\n",
                    new_path, files[qty]->d_name, files[qty]->d_name, pfs.value);
            }
        } else {
            sprintf(buffer, "=> %s%s/   <DIR> %s/\n", new_path, files[qty]->d_name, files[qty]->d_name);
        }

        if (write_ssl(ssl, buffer) <= 0) {
            goto EXIT;
        }
    }

EXIT:
    for(i=0; i<qty; i++) {
        free(files[i]);
    }
    free(files);
}

/* attempt to transfer the file over ssl in chunks */
/* non-zero return means error */
static int file_transfer(const char *path, SSL *ssl) {
    char *mime;
    char *buf;
    int err;
    FILE *f;
    size_t n;
    char header[256];

    if (!path || !ssl) {
        return 3;
    }

    err = 1;

    if (!(mime = (char *)mime_type(path))) {
        puts("mime error");
        return 1;
    }

    if (!(f = fopen(path, "r"))) {
        puts("fopen() error");
        return 2;
    }
    
    if (!(buf = malloc(GEM_XFER_CHUNK_SIZ + 1))) {
        goto EXIT;
    }

    sprintf(header, "20 %s\r\n", mime);
    if (write_ssl(ssl, header) <= 0) {
        goto EXIT;
    }
    
    while(!feof(f)) {
        n = fread(buf, 1, GEM_XFER_CHUNK_SIZ, f);
        if (n < GEM_XFER_CHUNK_SIZ && ferror(f)) {
            perror("fread()");
            goto EXIT;
        }

        if (SSL_write(ssl, buf, (int)n) <= 0) {
            perror("SSL_write()");
            goto EXIT;
        }
    }
    
    err = 0;

EXIT:
    free(buf);
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

    /* append user path to docroot */
    sprintf(buf, "%s%s", cfg.docroot, u->path);

    /* if file does not exist */
    if (!file_exists(buf)) {
        return RESP_FILE_NOT_FOUND;
    }

    /* if file is a directory */
    if (file_is_dir(buf)) {

        /* if it's a directory, and the path does not end in / */
        /* then append a / */
        if (buf[strlen(buf) - 1] != '/') {
            strcpy(buf + strlen(buf), "/");
        }

        /* if path contains index*/
        index = dir_has_index(buf);

        /* no index file present, and dir enumeration is enabled */
        if (!index && cfg.enumerate) {
            iterate_dir(buf, ssl);
            return 0;
        }

        /* index file is present so don't enum dir regardless */
        if (index) {
            strcpy(buf + strlen(buf), cfg.index);
        } else {
            /* no dir enum and no file to view, so send error 61 why not */
            resp_error(RESP_STATUS_CERT_NOT_AUTH, ssl);
            return 0;
        }
    }

    /* if file transfer failed */
    if (file_transfer(buf, ssl)) {
        return RESP_FILE_TRANSFER;
    }

    return 0;
}

/* write a gemini error code to client */
int resp_error(const char *code, SSL *ssl) {
    char buffer[16];
    sprintf(buffer, "%s\r\n", code);
    
    return write_ssl(ssl, buffer);
}

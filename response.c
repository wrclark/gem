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
#include "url.h"

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
    char escaped[2048];
    struct dirent **files;
    struct pfs_data pfs;
    int qty, i;
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

    /* write gemini response for gemtext document */
    if (write_ssl(ssl, "20 text/gemini\r\nIndex\n") <= 0) {
        goto EXIT;
    }

    for(i=0; i<qty; i++) {

        /* skip rel files . and .. */
        if (!strcmp(files[i]->d_name, ".") || !strcmp(files[i]->d_name, "..")) {
            continue;
        }

        sprintf(buffer, "%s%s", path, files[i]->d_name);
        if (!url_encode(buffer, escaped, 2048) != 0) {
            fprintf(stderr, "unable to url_encode \"%s\"\n", buffer);
            goto EXIT;
        }

        if (files[i]->d_type == DT_DIR) {
            sprintf(buffer, "=> %s/  %s/\n", escaped, files[i]->d_name);
        } else {
            size = filesize(buffer);
            pfs = pretty_filesize(size);
            sprintf(buffer, 
                pfs.type ? "=> %s  %s <%.2f %s>\n" : "=> %s  %s <%.0f %s>\n",
                escaped, files[i]->d_name, (double)pfs.value,
                pfs.type ? pfs.type : "B");
        }

        if (write_ssl(ssl, buffer) <= 0) {
            goto EXIT;
        }
    }

EXIT:
    while (qty--) {
        free(files[qty]);
    }
    free(files);
}

/* attempt to transfer the file over ssl in chunks */
/* non-zero return means error */
static int file_transfer(const char *path, SSL *ssl) {
    const char *mime;
    char *buf;
    int err;
    FILE *f;
    size_t n;
    char header[256];

    if (!path || !ssl) {
        return 3;
    }

    err = 1; /* default err */

    mime = mime_type(path);
    if (!mime) {
        puts("mime error");
        return 1;
    }

    f = fopen(path, "rb");
    if (!f) {
        puts("fopen() error");
        return 2;
    }
    
    buf = malloc(GEM_XFER_CHUNK_SIZ);
    if (!buf) {
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

    /* copy user-provided path to buf */
    sprintf(buf, "%s", u->path);

    /* if file does not exist */
    if (!file_exists(buf)) {
        return RESP_FILE_NOT_FOUND;
    }

    /* if file is a directory */
    if (file_is_dir(buf)) {

        /* Ensure there's enough space to append '/' */
        if (strlen(buf) < sizeof(buf) - 1) {
            if (buf[strlen(buf) - 1] != '/') {
                strcat(buf, "/");
            }
        } else {
            return RESP_FILE_PATH_TOO_LONG;
        }

        /* Check if directory contains an index file */
        index = dir_has_index(buf);

        /* no index file present, and dir enumeration is enabled */
        if (!index && cfg.enumerate) {
            iterate_dir(buf, ssl);
            return 0;
        }
        
        /* index file is present so don't enum dir regardless */
        if (index) {
            /* Ensure there's enough space to append index file */
            if (strlen(buf) + strlen(cfg.index) < sizeof(buf)) {
                strcat(buf, cfg.index);
            } else {
                return RESP_FILE_PATH_TOO_LONG;
            }
        } else {
            /* No directory enumeration and no index file, send error */
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

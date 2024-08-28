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
    char header[256] = {0};
    char meta[32] = {0};
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
    sprintf(header, "20 text/gemini");

    if (ENABLE_CHARSET_LOOKUP) {
        memset(meta, 0, 32);
        if (!file_read_dir_meta(path, ".charset", meta, 32)) {
            strcat(header, "; charset=");
            strcat(header, meta);
        } else if (GEM_USE_DEFAULT_META) {
            /* file read failed but can use default */
            strncpy(meta, GEM_DEFAULT_CHARSET, 32);
            strcat(header, "; charset=");
            strcat(header, meta);
        }
    }

    if (ENABLE_LANG_LOOKUP) {
        memset(meta, 0, 32);
        if (!file_read_dir_meta(path, ".lang", meta, 32)) {
            strcat(header, "; lang=");
            strcat(header, meta);
        } else if (GEM_USE_DEFAULT_META) {
            /* file read failed but can use default */
            strncpy(meta, GEM_DEFAULT_LANG, 32);
            strcat(header, "; lang=");
            strcat(header, meta);
        }
    }

    strcat(header, "\r\nIndex\n");

    if (write_ssl(ssl, header) <= 0) {
        goto EXIT;
    }

    for(i=0; i<qty; i++) {

        if (!ENUMERATE_DOT_FILES) {
            if (files[i]->d_name[0] == '.') {
                continue;
            }
        } else {
            /* skip rel files . and .. */
            if (!strcmp(files[i]->d_name, ".") || !strcmp(files[i]->d_name, "..")) {
                continue;
            }
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
    char header[256] = {0};
    char meta[32] = {0};
    const mime_t *mime;
    char *buf;
    int err;
    FILE *f;
    size_t n;

    err = 1; /* default err */


    if (!path || !ssl) {
        return 1;
    }

    if (!(mime = mime_type(path))) {
        return 1;
    }

    if (!(f = fopen(path, "rb"))) {
        return 1;
    }
    
    if (!(buf = malloc(GEM_XFER_CHUNK_SIZ))) {
        goto EXIT;
    }

    sprintf(header, "20 %s", mime->mime);

    if (ENABLE_CHARSET_LOOKUP && mime->type & MIME_TYPE_TEXT) {
        memset(meta, 0, 32);
        if (!file_read_dir_meta(path, ".charset", meta, 32)) {
            strcat(header, "; charset=");
            strcat(header, meta);
        } else if (GEM_USE_DEFAULT_META) {
            /* file read failed but can use default */
            strncpy(meta, GEM_DEFAULT_CHARSET, 32);
            strcat(header, "; charset=");
            strcat(header, meta);
        }
    }

    if (ENABLE_LANG_LOOKUP && mime->type & MIME_TYPE_TEXT) {
        memset(meta, 0, 32);
        if (!file_read_dir_meta(path, ".lang", meta, 32)) {
            strcat(header, "; lang=");
            strcat(header, meta);
        } else if (GEM_USE_DEFAULT_META) {
            /* file read failed but can use default */
            strncpy(meta, GEM_DEFAULT_LANG, 32);
            strcat(header, "; lang=");
            strcat(header, meta);
        }
    }

    strcat(header, "\r\n");
   
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

    if (!file_exists(buf)) {
        return RESP_FILE_NOT_FOUND;
    }

    if (file_is_dir(buf)) {

        /* ensure there's enough space to append '/' */
        if (strlen(buf) < sizeof(buf) - 1) {
            if (buf[strlen(buf) - 1] != '/') {
                strcat(buf, "/");
            }
        } else {
            return RESP_FILE_PATH_TOO_LONG;
        }

        /* check if directory contains an index file */
        index = dir_has_index(buf);

        /* no index file present, and dir enumeration is enabled */
        if (!index && cfg.enumerate) {
            iterate_dir(buf, ssl);
            return 0;
        }
        
        /* index file is present so don't enum dir regardless */
        if (index) {
            /* ensure there's enough space to append index file */
            if (strlen(buf) + strlen(cfg.index) < sizeof(buf)) {
                strcat(buf, cfg.index);
            } else {
                return RESP_FILE_PATH_TOO_LONG;
            }
        } else {
            /* no directory enumeration and no index file, send error */
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

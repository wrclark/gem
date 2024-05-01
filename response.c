#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

#include "response.h"
#include "mime.h"
#include "config.h"
#include "file.h"

static int write_ssl(SSL *ssl, const char *str) {
    return SSL_write(ssl, str, strlen(str));
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
    struct pfs_data pfs;

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
    strcpy(new_path, path + strlen(GEM_DOCROOT));

    write_ssl(ssl, "20 text/gemini\r\n");
    write_ssl(ssl, "## Directory listing\n");

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
            pfs = pretty_filesize(size);
            if (pfs.type) {
                sprintf(buffer, "=> gemini://" GEM_HOSTNAME "%s%s   <FILE> %s <%.2f %s>\n",
                    new_path, files[qty]->d_name, files[qty]->d_name, pfs.value, pfs.type);
            } else {
                sprintf(buffer, "=> gemini://" GEM_HOSTNAME "%s%s   <FILE> %s <%.0f B>\n",
                    new_path, files[qty]->d_name, files[qty]->d_name, pfs.value);
            }
            
        }
        write_ssl(ssl, buffer);
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
    
    if (!(buf = malloc(GEM_XFER_CHUNK_SIZ + 1))) {
        goto EXIT;
    }

    write_ssl(ssl, "20 ");
    write_ssl(ssl, mime);
    write_ssl(ssl, "\r\n");
    
    while(!feof(f)) {
        n = fread(buf, 1, GEM_XFER_CHUNK_SIZ, f);
        if (n < GEM_XFER_CHUNK_SIZ && ferror(f)) {
            perror("fread()");
            goto EXIT;
        }
        SSL_write(ssl, buf, n);
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

    sprintf(buf, "%s%s", GEM_DOCROOT, u->path);

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
        if (!index && GEM_LIST_DIR) {
            iterate_dir(buf, ssl);
            return 0;
        }

        /* index file is present so don't enum dir regardless */
        if (index) {
            strcpy(buf + strlen(buf), GEM_INDEX_FILE);
        } else {
            /* no dir enum and no file to view, so send error 61 why not */
            resp_error("61", ssl);
            return 0;
        }
    }

    /* if file transfer failed */
    if (file_transfer(buf, ssl)) {
        return RESP_FILE_TRANSFER;
    }

    return 0;
}

int resp_error(const char *code, SSL *ssl) {
    write_ssl(ssl, code);
    write_ssl(ssl, "\r\n");
    return 0;
}

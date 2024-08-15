#define _DEFAULT_SOURCE
#include <dirent.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "file.h"

extern struct gem_config cfg;

const char *pfs_kilo = "KB";
const char *pfs_mega = "MB";
const char *pfs_giga = "GB";

/* convert n bytes to a pretty file size format */
/* eg 3453554 -> "3.45 MB" */
struct pfs_data pretty_filesize(const size_t siz) {
    struct pfs_data p;
    
    if (siz >= 1000000000UL) {
        p.type = pfs_giga;
        p.value = (float)siz / 1000000000UL;
    } else if (siz >= 1000000UL) {
        p.type = pfs_mega;
        p.value = (float)siz / 1000000UL;
    } else if (siz >= 1000UL) {
        p.type = pfs_kilo;
        p.value = (float)siz / 1000UL;
    } else {
        p.type = NULL;
        p.value = (float)siz;
    }

    return p;
}

/* return file size */
size_t filesize(const char *path) {
    struct stat st;
    return (path && stat(path, &st) == 0) ? (size_t)st.st_size : 0;
}

/* check if a file is a directory */
int file_is_dir(const char *path) {
    struct stat st;
    return (path && stat(path, &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
}

/* check if a directory contains an index file */
/* as specified with -i flag, or default as "index.gmi" */
int dir_has_index(const char *path) {
    char buf[2048];

    if (!path || strlen(path) + strlen(cfg.index) + 2 >= sizeof(buf)) {
        return 0;
    }

    strcpy(buf, path);
    if (buf[strlen(buf) - 1] != '/') {
        strcat(buf, "/");
    }
    strcat(buf, cfg.index);

    return file_exists(buf);
}

/* check if file exists at all */
int file_exists(const char *path) {
    struct stat st;
    return (path && stat(path, &st) == 0);
}
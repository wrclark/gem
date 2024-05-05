#define _DEFAULT_SOURCE
#include <dirent.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "file.h"

const char *pfs_kilo = "KB";
const char *pfs_mega = "MB";
const char *pfs_giga = "GB";

/* convert n bytes to a pretty file size format */
/* eg 3453554 -> "3.45 MB" */
struct pfs_data pretty_filesize(const size_t siz) {
    struct pfs_data p;
    if (siz >= 1000000000UL) {
        p.type = (char *)pfs_giga;
        p.value = (float)(siz / 1000000000UL) + ((float)(siz % 1000000000UL))/((float)1000000000UL);
    } else if (siz >= 1000000UL) {
        p.type = (char *)pfs_mega;
        p.value = (float)(siz / 1000000UL) + ((float)(siz % 1000000UL))/((float)1000000UL);
    } else if (siz >= 1000UL) {
        p.type = (char *)pfs_kilo;
        p.value = (float)(siz / 1000UL) + ((float)(siz % 1000UL))/((float)1000UL);
    } else {
        p.type = NULL;
        p.value = (float)siz;
    }

    return p;
}

/* return file size */
size_t filesize(const char *path) {
    struct stat st;

    if (!path) {
        return 0;
    }

    if (stat(path, &st) != 0) {
        return 0;
    }

    return (size_t)(st.st_size);
}

/* check if a file is a directory */
/* non-zero return means that it is */
int file_is_dir(const char *path) {
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
int dir_has_index(const char *path) {
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

/* returns 1 if the file exists */
int file_exists(const char *path) {
    struct stat st;

    if (!path) {
        return 0;
    }

    return stat(path, &st) == 0;
}

#define _DEFAULT_SOURCE
#include <dirent.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

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

/* read N bytes from a file */
static int file_read_n(const char *path, char *buf, const size_t siz) {
    int fd;
    size_t bytes;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "unable to open %s\n", path);
        return 1;
    }

    bytes = (size_t)read(fd, buf, siz);
    if (bytes == (size_t)-1) {
        fprintf(stderr, "err reading %s\n", path);
        return 1;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "failed close file %s\n", path);
        return 1;
    }

    return 0;

}

/* takes a path ex: "/main/en-US/hello.gmi"        */
/* finds the base directory of the file            */
/* ex: "/main/en-US/hello.gmi" -> "/main/en-US/"   */
/* ex: "/main/asdsa.txt" -> "/main/"               */
/* ex: "/index.gmi" -> "/"                         */
/* ex: "/" -> "/"                                  */
/* checks if this directory contains a file (arg2) */
/* if so, writes <=n bytes of this file to a dest buffer and returns 0 */
/* if not, returns 1 */
int file_read_dir_meta(const char *path, const char *file, char *buf, const size_t bufsiz) {
    char dir_buf[512]= {0};
    size_t dir_length, i;
    char *slash;

    if (!path || !file || !buf) {
        return 1;
    }

    if (!strcmp(path, "/") || path[strlen(path)-1] == '/') {
        strncpy(dir_buf, path, 512);
        strcat(dir_buf, file);
    } else {
        slash = strrchr(path, '/');
        dir_length = (size_t)(slash - path +1);
        strncpy(dir_buf, path, dir_length);
        strcat(dir_buf, file);
    }

    /* check if file exists */
    if (!file_exists(dir_buf)) {
        return 1;
    }

    if (file_read_n(dir_buf, buf, bufsiz) != 0) {
        return 1;
    }

    /* trim newline at the end */
    for(i=bufsiz-1; i; i--) {
        if (buf[i] == '\n') {
            buf[i] = '\0';
        }
    }

    return 0;
}
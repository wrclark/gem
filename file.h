#ifndef FILE_H
#define FILE_H

/* "pretty file size" */
struct pfs_data {
    char *type;
    float value;
};

struct pfs_data pretty_filesize(size_t siz);
size_t filesize(const char *path);
int file_is_dir(const char *path);
int dir_has_index(const char *path);
int file_exists(const char *path);

#endif
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "file.h"

/* Validate that the document root path exists and is a directory */
static int validate_docroot(const char *path) {
    if (path == NULL || strlen(path) == 0) {
        fprintf(stderr, "Document root path is NULL or empty.\n");
        return 0;
    }

    if (!file_exists(path)) {
        fprintf(stderr, "Document root path does not exist: %s\n", path);
        return 0;
    }

    if (!file_is_dir(path)) {
        fprintf(stderr, "Document root path is not a directory: %s\n", path);
        return 0;
    }

    return 1;
}

/* Validate the index file name */
static int validate_index_file(const char *index) {
    if (index == NULL || strlen(index) == 0) {
        fprintf(stderr, "Index file name is NULL or empty.\n");
        return 0;
    }

    if (strcmp(index, ".") == 0 || strcmp(index, "..") == 0) {
        fprintf(stderr, "Index file name cannot be '.' or '..'.\n");
        return 0;
    }

    return 1;
}

/* Validate the hostname length */
static int validate_hostname(const char *host) {
    if (host == NULL || strlen(host) == 0) {
        fprintf(stderr, "Hostname is NULL or empty.\n");
        return 0;
    }

    if (strlen(host) < 5) {  /* Assuming minimum hostname length of 5 for example */
        fprintf(stderr, "Hostname is too short: %s\n", host);
        return 0;
    }

    return 1;
}

/* Validate the port number */
static int validate_port(int port) {
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Port number is out of valid range (1-65535): %d\n", port);
        return 0;
    }

    return 1;
}

/* Validate the user-provided configuration */
int cfg_validate(struct gem_config *cfg) {
    if (cfg == NULL) {
        fprintf(stderr, "Configuration is NULL.\n");
        return 1;
    }

    if (!validate_docroot(cfg->docroot)) {
        return 1;
    }

    if (!validate_index_file(cfg->index)) {
        return 1;
    }

    if (!validate_hostname(cfg->hostname)) {
        return 1;
    }

    if (!validate_port(cfg->port)) {
        return 1;
    }

    return 0;
}

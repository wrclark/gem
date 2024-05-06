#include <stdio.h>
#include <string.h>

#include "config.h"
#include "file.h"


static int validate_docroot(const char *path) {
    if (!path) {
        return 0;
    }

    return file_exists(path) && file_is_dir(path);
}

static int validate_index_file(const char *index) {
    if (!index) {
        return 0;
    }

    return (strlen(index) > 0) && strcmp(index, ".") && strcmp(index, "..");
}

static int validate_hostname(const char *host) {
    if (!host) {
        return 0;
    }

    return strlen(host) > 4;
}

static int validate_port(int port) {
    return port > 0;
}

/* try to validate user provided config */
/* non-zero return means error */
int cfg_validate(struct gem_config *cfg) {

    if (!cfg) {
        return 1;
    }

    if (!validate_docroot(cfg->docroot)) {
        fprintf(stderr, "invalid doc root path\n");
        return 1;
    }

    if (!validate_index_file(cfg->index)) {
        fprintf(stderr, "invalid index file\n");
        return 1;
    }

    if (!validate_hostname(cfg->hostname)) {
        fprintf(stderr, "invalid hostname\n");
        return 1;
    }

    if (!validate_port(cfg->port)) {
        fprintf(stderr, "invalid port\n");
        return 1;
    }


    return 0;
}
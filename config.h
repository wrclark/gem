#ifndef CONFIG_H
#define CONFIG_H

/* SSL/TLS */
#define PUBLIC_KEY "tls/server.crt"
#define PRIVATE_KEY "tls/server.key"

/* maximum size of a request */
#define GEM_URI_MAXSIZ 1024

/* malloc chunk size for transferring files */
#define GEM_XFER_CHUNK_SIZ (1 << 14) /* 16KB */

#define GEM_CFG_SIZ 512

struct gem_config {
    char hostname[GEM_CFG_SIZ + 1];
    char docroot[GEM_CFG_SIZ + 1];
    char index[GEM_CFG_SIZ + 1];
    int port;
    int enumerate;
    int diffhost;
};

int cfg_validate(struct gem_config *cfg);

#endif
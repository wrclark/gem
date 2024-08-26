#ifndef CONFIG_H
#define CONFIG_H

/* SSL/TLS */
#define PUBLIC_KEY_PATH  "tls/server.crt"
#define PRIVATE_KEY_PATH "tls/server.key"

/* maximum size of a request */
/* > 1024 because of URL encoding */
#define GEM_URI_MAXSIZ 4096

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
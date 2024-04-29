#ifndef REQUEST_H
#define REQUEST_H

#define REQUEST_MAX_SCHEME     16
#define REQUEST_MAX_DOMAIN     128
#define REQUEST_MAX_PORT       5
#define REQUEST_MAX_PATH       256
#define DECODER_BUFSIZ 512 /* ensure this is the largest */

#define REQUEST_ERR_SCHEME (1 << 0)
#define REQUEST_ERR_DOMAIN (1 << 1)
#define REQUEST_ERR_PORT   (1 << 2)
#define REQUEST_ERR_PATH   (1 << 3)
#define REQUEST_ERR_TERM   (1 << 4)

struct gem_uri{
    char scheme[REQUEST_MAX_SCHEME + 1];
    char domain[REQUEST_MAX_DOMAIN + 1];
    char port[REQUEST_MAX_PORT + 1];
    char path[REQUEST_MAX_PATH + 1];
    int error; 
};

void request_parse(const char *request, struct gem_uri *u);
void request_validate_uri(struct gem_uri *u);
void request_print_uri(struct gem_uri *u);

#endif

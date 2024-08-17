#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "request.h"
#include "url.h"

extern struct gem_config cfg;

#define STATE_SCHEME 0
#define STATE_DOMAIN 1
#define STATE_PORT 2
#define STATE_PATH 3

/*  start by checking that the request string ends with \r\n
   if it does, reduce its length

  decompose a URI into its (relevant) constituent parts
   scheme://authority/path
   authority is domain followed by an optional port
   ie http://example.com
      http://example.com:1337
      http://example.com:1337/cool/picture.png
   ->
   --> scheme: http
   --> domain: example.com
   --> port: 1337 ("" if unspecified)
   --> path: /cool/picture.png
  
   throw away request if error != 0
*/
void request_parse(const char *uri, struct gem_uri *u) {
    char request[1024 + 1] = {0};
    char buffer[DECODER_BUFSIZ] = {0};
    int bufidx = 0;
    int state = STATE_SCHEME;
    int i = 0;
    char *p;

    if (!uri || !u) {
        return;
    }

    if (!url_decode(uri, request, 1024) != 0) {
        u->error |= REQUEST_ERR_TERM;
        printf("error decoding url: %s\n", uri);
        return;
    }

    /* request does not end with \r\n, abort .. */
    if (strcmp("\r\n", &request[strlen(request) - 2])) {
        u->error |= REQUEST_ERR_TERM;
        return;
    }

    p = request;
    while (*p++) {
        if (*p == '\r' || *p == '\n') {
            *p = '\0';
            break;
        }
    }

    while (request[i]) {
START:
        switch (state) {
        case STATE_SCHEME:
            if (bufidx >= REQUEST_MAX_SCHEME) {
                u->error |= REQUEST_ERR_SCHEME;
                return;
            }
            if (request[i] == ':') {
                state = STATE_DOMAIN;
                /* I have a feeling this could crash */
                if (!(request[i + 1] == request[i + 2] && request[i + 2] == '/')) {
                    u->error |= REQUEST_ERR_SCHEME;
                    return;
                }
                i += 3;
                strcpy(u->scheme, buffer);
                memset(buffer, 0, DECODER_BUFSIZ);
                bufidx = 0;
                goto START;
            }
            break;
        case STATE_DOMAIN:
            if (bufidx >= REQUEST_MAX_DOMAIN) {
                u->error |= REQUEST_ERR_DOMAIN;
                return;
            }
            if (request[i] == ':') {
                state = STATE_PORT;
                i += 1;
                strcpy(u->domain, buffer);
                memset(buffer, 0, DECODER_BUFSIZ);
                bufidx = 0;
                goto START;
            }

            if (request[i] == '/') {
                state = STATE_PATH;
                strcpy(u->domain, buffer);
                memset(buffer, 0, DECODER_BUFSIZ);
                bufidx = 0;
                goto START;
            }
            break;
        case STATE_PORT:
            if (bufidx >= REQUEST_MAX_PORT) {
                u->error |= REQUEST_ERR_PORT;
                return;
            }

            if (request[i] == '/') {
                state = STATE_PATH;
                strcpy(u->port, buffer);
                memset(buffer, 0, DECODER_BUFSIZ);
                bufidx = 0;
                goto START;
            }
            break;
        case STATE_PATH:
            if (bufidx >= REQUEST_MAX_PATH) {
                u->error |= REQUEST_ERR_PATH;
                return;
            }
            if (request[i] == '?') {
                strcpy(u->path, buffer);
                memset(buffer, 0, DECODER_BUFSIZ);
                bufidx = 0;
                return;
            }
            break;
        }

        buffer[bufidx++] = request[i];
        i++;
    }

/*  reading the URI can terminate in the states domain, port or path
    this does not change the state previously set */

    switch (state) {
    case STATE_DOMAIN:
        if (bufidx >= REQUEST_MAX_DOMAIN) {
            u->error |= REQUEST_ERR_DOMAIN;
            return;
        }
        strcpy(u->domain, buffer);
        break;
    case STATE_PORT:
        if (bufidx >= REQUEST_MAX_PORT) {
            u->error |= REQUEST_ERR_PORT;
            return;
        }
        strcpy(u->port, buffer);
        break;
    case STATE_PATH:
        if (bufidx >= REQUEST_MAX_PATH) {
            u->error |= REQUEST_ERR_PATH;
            return;
        }
        strcpy(u->path, buffer);
        break;
    }

    /* scheme should never be empty */
    if (!strlen(u->scheme)) u->error |= REQUEST_ERR_SCHEME;

    /* domain should never be empty */
    if (!strlen(u->domain)) u->error |= REQUEST_ERR_DOMAIN;
}

/* Check path for "./" and "../" and normalize empty paths to "/" */
/* Set the error field accordingly */
void request_validate_uri(struct gem_uri *u) {
    if (!u) {
        return;
    }

    /* Check for "./" and "../" in the path */
    if (strstr(u->path, "../") || strstr(u->path, "./")) {
        u->error |= REQUEST_ERR_PATH;
        return;
    }

    /* Change an empty path to "/" */
    if (!strlen(u->path)) {
        strncpy(u->path, "/", REQUEST_MAX_PATH - 1);
        u->path[REQUEST_MAX_PATH - 1] = '\0';
    }

    /* Only allow the scheme to be "gemini" */
    if (memcmp("gemini", u->scheme, sizeof("gemini") - 1) || u->scheme[sizeof("gemini") - 1] != '\0') {
        u->error |= REQUEST_ERR_WRONG_SCHEME;
        return;
    }

    /* Only allow the hostname if diffhost is not enabled */
    if (!cfg.diffhost) {
        if (strncmp(cfg.hostname, u->domain, REQUEST_MAX_DOMAIN)) {
            u->error |= REQUEST_ERR_WRONG_DOMAIN;
            return;
        }
    }

    /* Only allow the correct port */
    if (strlen(u->port) > 0 && atoi(u->port) != cfg.port) {
        u->error |= REQUEST_ERR_PORT;
        return;
    }
}

void request_print_uri(struct gem_uri *u) {
    printf("scheme: %s\n"
           "domain: %s\n"
           "port: %s\n"
           "path: %s\n"
           "error: %d\n\n",
           u->scheme, u->domain, u->port, u->path, u->error);
}

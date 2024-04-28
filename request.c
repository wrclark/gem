#include <stdio.h>
#include <string.h>

#include "request.h"
#include "config.h"

/* 0 if scheme is gopher, http or https */
static int scheme_proxy(struct request *r) {
    return strncmp(r->data, "gopher://", 9)
            || strncmp(r->data, "http://", 7)
            || strncmp(r->data, "https://", 8);
}

/* if the request contains invalid characters */
static int invalid_characters(struct request *r) {
    int i;

    for(i=0; i<r->size; i++) {
        if (r->data[i] == '\\') {
            return 1;
        }
    }

    return 0;
}

/* if the request is terminated with \r\n */
static int rn_terminated(struct request *r) {
    return strncmp(&(r->data[r->size - 2]), "\r\n", 2);
}

/* 0 if the hostname after "gemini://" is valid */
static int valid_hostname(struct request *r) {
    return strncmp(r->data + 9, GEM_HOSTNAME, strlen(GEM_HOSTNAME));
}

/* definitely too small */
static int request_too_small(struct request *r) {
    return r->size <= 10;
}

/* 0 if the request begins with "gemini://" */
static int scheme_missing(struct request *r) {
    return strncmp(r->data, "gemini://", 9);
}

/* check if a request string is valid.
 * starts with gemini://$HOSTNAME/(.*)
 * may contain port also: gemini://$HOSTNAME:1965
 * and the final 2 characters are CR LF, like in http
 * TODO: ignore conf'd hostname on local ip ranges */
int req_valid(struct request *req) {

    if (request_too_small(req)) {
        return 1;
    }

    if (scheme_missing(req)) {
        if (scheme_proxy(req)) {
            return 10;
        }
        return 2;
    }

    if (valid_hostname(req)) {
        return 3;
    }

    if (rn_terminated(req)) {
        return 4;
    }

    if (invalid_characters(req)) {
        return 5;
    }

    return 0;
}

/* get the resource requested in the validated request */
int req_resource(struct request *req, struct resource *r) {
    char *p;

    strcpy(r->data, req->data + strlen(GEM_HOSTNAME) + 9);
    r->size = strlen(r->data);

    p = &(r->data[r->size - 1]);

    /* clean up trailing \r and \n */
    while(*p == '\r' || *p == '\n') {
        *p = '\0';
        r->size--;
        p--;
    }

    /* remove leading "../" eg: "../../../../etc/passwd" */
    /* just stop and send a FAIL status response */
    if (strstr(r->data, "../") || strstr(r->data, "./")) {
        return 1;
    }

    return 0;
}

/* links that end in a / should have that / replaced with */
/* "/index.gmi" */
/* if the RR is just "" then replace it with "/index.gmi" */
int req_check_index(struct resource *r) {
    if (r->size < 1) {
        r->data[r->size++] = '/';
    }

    if (r->data[r->size-1] == '/') {
        /* prob always fits */
        strcpy(&(r->data[r->size]), "index.gmi");
        r->size += strlen("index.gmi");
    }

    return 0;
}

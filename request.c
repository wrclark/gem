#include "request.h"
#include "config.h"

#include <string.h>

/* check if a request string is valid.
 * starts with gemini://$HOSTNAME/(.*)
 * and the final 2 characters are CR LF, like in http
 * TODO: ignore conf'd hostname on local ip ranges */
int req_valid(struct request *req) {

    if (req->size < 9 + strlen(GEM_HOSTNAME) + 2) {
        return 1;
    }

    if (strncmp(req->data, "gemini://", 9)) {
        return 2;
    }

    if (strncmp(req->data + 9, GEM_HOSTNAME, strlen(GEM_HOSTNAME))) {
        return 3;
    }

    if (strncmp(&(req->data[req->size - 2]), "\r\n", 2)) {
        return 4;
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

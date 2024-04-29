#ifndef RESPONSE_H
#define RESPONSE_H

#define RESP_STATUS_INPUT_EXP     "10"
#define RESP_STATUS_SUCCESS       "20"
#define RESP_STATUS_REDIRECT      "30"
#define RESP_STATUS_TEMPFAIL      "40"
#define RESP_STATUS_PERMFAIL      "50"
#define RESP_STATUS_NOT_FOUND     "51"
#define RESP_STATUS_PROXY_REFUSED "53"
#define RESP_STATUS_BAD_REQUEST   "59"
#define RESP_STATUS_CERT          "60"

#include <openssl/ssl.h>
#include "request.h"

int resp_file_exists(struct gem_uri *u);
int resp_file_transfer(struct gem_uri *u, SSL *ssl);
int resp_redirect(const char *path, SSL *ssl);
int resp_error(const char *code, SSL *ssl);

#endif

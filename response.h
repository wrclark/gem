#ifndef RESPONSE_H
#define RESPONSE_H

#define RESP_STATUS_INPUT_EXP     "10"
#define RESP_STATUS_SUCCESS       "20"
#define RESP_STATUS_REDIRECT      "30"
#define RESP_STATUS_TEMPFAIL      "40"
#define RESP_STATUS_PERMFAIL      "50"
#define RESP_STATUS_NOT_FOUND     "51"
#define RESP_STATUS_GONE          "52"
#define RESP_STATUS_PROXY_REFUSED "53"
#define RESP_STATUS_BAD_REQUEST   "59"
#define RESP_STATUS_CERT          "60"
#define RESP_STATUS_CERT_NOT_AUTH "61"

#define RESP_FILE_NOT_FOUND     (1 << 0)
#define RESP_FILE_TRANSFER      (1 << 1)
#define RESP_FILE_PATH_TOO_LONG (1 << 2)

#include <openssl/ssl.h>
#include "request.h"

int resp_serve_file(struct gem_uri *u, SSL *ssl);
int resp_error(const char *code, SSL *ssl);

#endif


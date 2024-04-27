#ifndef RESPONSE_H
#define RESPONSE_H

#define RESP_STATUS_INPUT_EXP 10
#define RESP_STATUS_SUCCESS   20
#define RESP_STATUS_REDIRECT  30
#define RESP_STATUS_TEMPFAIL  40
#define RESP_STATUS_PERMFAIL  50
#define RESP_STATUS_CERT      60

#include <openssl/ssl.h>
#include "request.h"

int resp_file_exists(struct resource *res);
int resp_file_transfer(struct resource *res, SSL *ssl);

#endif

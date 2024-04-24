#ifndef RESPONSE_H
#define RESPONSE_H

#define RESP_STATUS_INPUT_EXP 10
#define RESP_STATUS_SUCCESS   20
#define RESP_STATUS_REDIRECT  30
#define RESP_STATUS_TEMPFAIL  40
#define RESP_STATUS_PERMFAIL  50
#define RESP_STATUS_CERT      60

#include "request.h"

int resp_resource_exists(struct resource *r);

#endif

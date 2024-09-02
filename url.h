#ifndef URL_H
#define URL_H

#include <stdio.h>
#include "request.h"

int url_encode(const char *user, char *buffer, const size_t size);
int url_decode(const char *user, char *buffer, const size_t size);

#endif


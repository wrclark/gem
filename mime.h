#ifndef MIME_H
#define MIME_H

#include "response.h"

char *mime_type_by_ext(char *ext);
char *mime_type(const char *path);

#endif

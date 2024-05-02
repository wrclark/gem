#ifndef MIME_H
#define MIME_H

#include "response.h"

const char *mime_type_by_ext(const char *ext);
const char *mime_type(const char *path);

#endif

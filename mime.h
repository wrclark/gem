#ifndef MIME_H
#define MIME_H

#include "response.h"

#define MIME_TYPE_TEXT   (1 << 0)
#define MIME_TYPE_IMAGE  (1 << 1)
#define MIME_TYPE_VIDEO  (1 << 2)
#define MIME_TYPE_AUDIO  (1 << 3)
#define MIME_TYPE_DOC    (1 << 4)
#define MIME_TYPE_OTHER  (1 << 5)

typedef struct {
    const char *ext;
    const char *mime;
    unsigned char type;
} mime_t;

const mime_t *mime_type_by_ext(const char *ext);
const mime_t *mime_type(const char *path);

#endif


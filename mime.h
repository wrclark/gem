/*
 * This file is part of gem.
 *
 * Copyright (C) 2025 William Clark
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */


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


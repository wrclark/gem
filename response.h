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


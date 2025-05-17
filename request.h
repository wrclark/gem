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


#ifndef REQUEST_H
#define REQUEST_H

/* choose what you want */
#define REQUEST_MAX_SCHEME     16
#define REQUEST_MAX_DOMAIN     128
#define REQUEST_MAX_PORT       5
#define REQUEST_MAX_PATH       256
#define DECODER_BUFSIZ         512 /* ensure this is the largest */

/* error parsing scheme */
#define REQUEST_ERR_SCHEME        (1 << 0)
/* error parsing domain */
#define REQUEST_ERR_DOMAIN        (1 << 1)
/* error parsing port */
#define REQUEST_ERR_PORT          (1 << 2)
/* error parsing path */
#define REQUEST_ERR_PATH          (1 << 3)
/* request contains no \r\n terminator */
#define REQUEST_ERR_TERM          (1 << 4)
/* scheme is OK but not accepted (proxying) */
#define REQUEST_ERR_WRONG_SCHEME  (1 << 5)
/* domain is OK but not accepted (proxying) */
#define REQUEST_ERR_WRONG_DOMAIN  (1 << 6)

struct gem_uri{
    char scheme[REQUEST_MAX_SCHEME + 1];
    char domain[REQUEST_MAX_DOMAIN + 1];
    char port[REQUEST_MAX_PORT + 1];
    char path[REQUEST_MAX_PATH + 1];
    int error; 
};

void request_parse(const char *request, struct gem_uri *u);
void request_validate_uri(struct gem_uri *u);
void request_print_uri(struct gem_uri *u);

#endif


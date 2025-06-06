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


#ifndef CONFIG_H
#define CONFIG_H

/* SSL/TLS */
#define PUBLIC_KEY_PATH  "tls/server.crt"
#define PRIVATE_KEY_PATH "tls/server.key"

/* REQUESTS */
/* should files whose names begin with '.' be enumerated  */
/* does not apply to relative fs files "." and ".."       */
/* ^^^ they are not enumerated regardless of this setting */
#define ENUMERATE_DOT_FILES 0
/* search current directory for ".lang" and ".charset" files */
/* that are included in the META gemini header. */
/* Ex: "20 text/gemini; charset=utf-8; lang=en" */
#define ENABLE_CHARSET_LOOKUP 1
#define ENABLE_LANG_LOOKUP 1

/* when doing a charset/lang lookup.. */
/* if no .lang/charset file is found, send out defaults ? */
/* only for text file mime types !! */
#define GEM_USE_DEFAULT_META 1
#define GEM_DEFAULT_CHARSET "utf-8"
#define GEM_DEFAULT_LANG "en"

/* maximum size of a request */
/* > 1024 because of URL encoding */
#define GEM_URI_MAXSIZ 4096

/* malloc chunk size for transferring files */
#define GEM_XFER_CHUNK_SIZ (1 << 14) /* 16KB */

#define GEM_CFG_SIZ 512

struct gem_config {
    char hostname[GEM_CFG_SIZ + 1];
    char docroot[GEM_CFG_SIZ + 1];
    char index[GEM_CFG_SIZ + 1];
    char key_path[GEM_CFG_SIZ + 1];
    char crt_path[GEM_CFG_SIZ + 1];
    int port;
    int enumerate;
    int diffhost;
    int verbose;
};

int cfg_validate(struct gem_config *cfg);

#endif


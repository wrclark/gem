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


#ifndef FILE_H
#define FILE_H

/* "pretty file size" */
struct pfs_data {
    const char *type;
    float value;
};

struct pfs_data pretty_filesize(size_t siz);
size_t filesize(const char *path);
int file_is_dir(const char *path);
int dir_has_index(const char *path);
int file_exists(const char *path);
int file_read_dir_meta(const char *path, const char *file, char *buf, const size_t bufsiz);


#endif


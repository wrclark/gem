#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "url.h"


/* encode a string with "problematic" characters in UTF-8 special characters */
int url_encode(const char *user, char *buffer, const size_t size) {
    const char *hex = "0123456789ABCDEF";
    size_t i, j;

    if (!user || !buffer || size == 0) {
        return -1;
    }

    for (i = 0, j = 0; user[i] != '\0' && j < size - 1; i++) {
        char c = user[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '-' || c == '_' || 
            c == '.' || c == '~' || c == '/') {
            buffer[j++] = c;
        } else if (j < size - 3) { /* ensure enough space for encoding */
            buffer[j++] = '%';
            buffer[j++] = hex[(unsigned char)c >> 4];
            buffer[j++] = hex[(unsigned char)c & 0x0F];
        } else {
            break; /* no space for encoding, stop the loop */
        }
    }

    buffer[j] = '\0'; /* null-terminate the encoded string */
    return j < size;
}

/* decode a url encoded string */
int url_decode(const char *user, char *buffer, const size_t size) {
    size_t i, j;
    char hex[3];

    if (!user || !buffer || size == 0) {
        return -1;
    }

    for (i = 0, j = 0; user[i] != '\0' && j < size - 1; i++) {
        if (user[i] == '%' && isxdigit(user[i + 1]) && isxdigit(user[i + 2])) {
            hex[0] = user[i + 1];
            hex[1] = user[i + 2];
            hex[2] = 0;
            buffer[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (user[i] == '+') {
            buffer[j++] = ' ';
        } else {
            buffer[j++] = user[i];
        }
    }

    buffer[j] = '\0'; /* null-terminate the decoded string */
    return j < size;
}

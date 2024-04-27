#include <openssl/ssl.h>
#include "response.h"
#include "mime.h"

/* determine if the file exists within the docroot */
int resp_file_exists(struct resource *res) {
    return 0;
}

/* attempt to (in chunks*) transfer the requested file */
int resp_file_transfer(struct resource *res, SSL *ssl) {
    /* determine mime type here */
    return 0;
}

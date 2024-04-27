#ifndef CONFIG_H
#define CONFIG_H

/* SSL/TLS */
#define PUBLIC_KEY "tls/server.crt"
#define PRIVATE_KEY "tls/server.key"

/* default gemini port */
#define GEM_PORT 1965

/* change to example.com */
#define GEM_HOSTNAME "localhost"

/* directory from which the .gmi files are served */
/* like /var/www/ */
#define GEM_DOCROOT "capsule/"

#endif
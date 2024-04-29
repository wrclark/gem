#ifndef CONFIG_H
#define CONFIG_H

/* SSL/TLS */
#define PUBLIC_KEY "tls/server.crt"
#define PRIVATE_KEY "tls/server.key"

/* default gemini port */
#define GEM_PORT 1965

/* change to example.com */
#define GEM_HOSTNAME "localhost"
/* set to 1 to only allow the specified hostname in requests */
#define GEM_ONLY_HOSTNAME 1

/* directory from which the .gmi files are served */
/* like /var/www */
#define GEM_DOCROOT "capsule"

#define GEM_URI_MAXSIZ 1024

#endif
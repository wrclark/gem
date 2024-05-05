# gem

gemini server with TLS and script for generating working TLS certs

Supports 
- mime types
- directory listing
- chunked file transfer
- passes most gemini-diagnostics tests

To build you may need to install `libssl-dev`

How to run:
```sh
$ make ssl
$ make
$ ./gem
```

To use your own domain name you have to replace `/CN=localhost` in `ssl` make target to your domain.

Eg `example.com` becomes `/CN=example.com`, or just copy the cert made by certbot (Let's Encrypt/EFF).

Also change `GEM_HOSTNAME` in `config.h`. But, for testing `GEM_ONLY_HOSTNAME` can be set to `0` and then you don't have to do anything.

Tip: don't try to serve files with symbols like a space " ", #, & etc. It doesn't work with most browsers.

Building on Raspberry Pi? remove `-fcf-protection` cflag

Great development tool:
```
https://github.com/michael-lazar/gemini-diagnostics
```

Test gemini client:
```sh
echo "gemini://localhost" | openssl s_client -quiet -crlf -connect localhost:1965
```
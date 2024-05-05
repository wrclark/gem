# gem

gemini server with TLS and script for generating working TLS certs

To build you may need to install `libssl-dev`

Make changes to `config.h`

How to run:
```sh
$ make ssl
$ make
$ ./gem
```

Test gemini client:
```sh
echo "gemini://localhost" | openssl s_client -quiet -crlf -connect localhost:1965
```

Great development tool:
```
https://github.com/michael-lazar/gemini-diagnostics
```

Supports 
- mime types
- directory listing
- chunked file transfer
- passes most gemini-diagnostics tests

To use your own domain name you have to replace `/CN=localhost` in `ssl` make target to your domain, eg `example.com` or copy the cert made by certbot (Let's Encrypt/EFF).
Also change `GEM_HOSTNAME` in `config.h`.

Tip: don't try to serve files with symbols like a space " ", #, & etc. It doesn't work with most browsers.

Building on Raspberry Pi? remove `-fcf-protection` cflag
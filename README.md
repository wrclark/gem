# gem

> gemini server with TLS and script for generating working TLS certs

Supports 
- mime types
- directory listing
- chunked file transfer
- passes most gemini-diagnostics tests

To build you may need to install `libssl-dev`

```sh
$ make ssl
$ make
```
### Program options
```
-h [HOSTNAME]   ex: -h "example.com"   (localhost default)
-p [PORT]       ex: -p 1965            (default)
-d [DOC ROOT]   ex: -d "/var/gemini"
-i [INDEX FILE] ex: -i "index.gmi"     (default)
-e  enumerate directories without an index file
-a  permit requests with a different hostname
```

Run with `./gem -d capsule`

To use your own domain name you have to replace `/CN=localhost` in the `ssl` make target to your domain: eg `example.com` becomes `/CN=example.com`.

Then you must also specify the domain as the hostname when running the program: `./gem -h "example.com" -d capsule`

### misc

Tip: don't try to serve files with symbols like a space " ", #, & etc. It doesn't work with most browsers.

Building on Raspberry Pi? remove the `-fcf-protection` CFLAG

Great development tool:
```
https://github.com/michael-lazar/gemini-diagnostics
```

Test gemini client:
```sh
echo "gemini://localhost" | openssl s_client -quiet -crlf -connect localhost:1965
```
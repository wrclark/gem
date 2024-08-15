# gem

> gemini server with TLS and script for generating working TLS certs

Supports 
- mime types
- directory listing
- chunked file transfer
- passes most gemini-diagnostics tests

![Directory listing as seen in Lagrange](.gitea/lagrange.png)

### build
To build you may need to install `libssl-dev`

```sh
$ sudo apt install libssl-dev
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

Run with `./gem -d capsule -e`

#### SSL cert
To use your own domain name you have to replace `/CN=localhost` in the `ssl` make target to your domain: eg `example.com` => `/CN=example.com`.

Then you must also specify the domain as the hostname when running the program:
```sh
./gem -h "example.com" -d capsule -e
```

The `-a` flag can be useful for accessing the server over IP (perhaps over LAN) without a DNS name. The `-d` flag must always be specified.

#### Raspberry Pi
Makefile now conditionally includes flags that don't seem to work on the Pi. Below is an example to easily get started, just replace the IP.
```sh
$ sudo apt update && sudo apt install -y libssl-dev
$ make ssl
$ make
$ ./gem -d capsule -a
```

## Misc

Great development tool:
```
https://github.com/michael-lazar/gemini-diagnostics
```

Test gemini client:
```sh
echo "gemini://localhost" | openssl s_client -quiet -crlf -connect localhost:1965
```
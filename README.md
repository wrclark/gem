# gem

gemini server with TLS and script for generating working TLS certs

Make changes to `config.h`

How to run:
```sh
$ sudo apt install libssl-dev
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
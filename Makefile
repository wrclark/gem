SHELL:=/usr/bin/bash
CC=gcc
CFLAGS=-std=c89 -O2 -W -Wall -Wextra -pedantic -I.
CFILES=$(wildcard *.c)
BIN=gem

all:
	$(CC) $(CFLAGS) $(CFILES) -o $(BIN) -lssl -lcrypto

ssl:
	mkdir -p tls
	openssl genrsa -aes256 -passout pass:gemini123 -out tls/server.pass.key 4096
	openssl rsa -passin pass:gemini123 -in tls/server.pass.key -out tls/server.key
	@rm tls/server.pass.key
	openssl req -new -key tls/server.key -out tls/server.csr
	openssl x509 -req -sha256 -days 3650 -in tls/server.csr -signkey tls/server.key -out tls/server.crt




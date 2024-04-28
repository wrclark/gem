SHELL:=/usr/bin/bash
CC=gcc
CFLAGS=-std=c89 -O2 -W -Wall -Wextra -pedantic -I.
#CFLAGS=-std=c89 -O0 -ggdb3 -W -Wall -Wextra -pedantic -I.
CFILES=$(wildcard *.c)
BIN=gem

all:
	$(CC) $(CFLAGS) $(CFILES) -o $(BIN) -lssl -lcrypto

ssl:
	mkdir -p tls
	openssl req -x509 -newkey rsa:4096 -keyout tls/server.key -out tls/server.crt -sha256 -days 3650 -nodes -subj '/CN=localhost'

clean:
	rm -rf $(BIN) tls/
SHELL := /usr/bin/bash
CC = gcc
CFLAGS = -std=c89 -O3 -W -Wall -Wextra -pedantic -I.
CFLAGS += -march=native -pipe -D_FORTIFY_SOURCE=2 -fstack-protector-strong
CFLAGS += -fstack-clash-protection -fcf-protection
CFLAGS += -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes
CFLAGS += -Wwrite-strings -Wconversion -Wunreachable-code
CFLAGS += -flto -funroll-loops -fPIE -pie -Wl,-z,relro,-z,now
CFLAGS += -fno-strict-aliasing

# Additional warnings and static analysis
CFLAGS += -Wformat=2 -Wformat-overflow=2 -Wformat-signedness
CFLAGS += -Wmissing-prototypes -Wmissing-declarations
CFLAGS += -Wredundant-decls -Wstrict-overflow=5 -Winline
CFLAGS += -Wlogical-op -Wswitch-enum
CFLAGS += -Wduplicated-cond -Wduplicated-branches -Wnull-dereference
CFLAGS += -Wfloat-equal -Wundef -Wbad-function-cast
CFLAGS += -Wcast-qual -Wold-style-definition -Wstrict-aliasing=2
CFLAGS += -Wdouble-promotion -Wimplicit-fallthrough=5
CFLAGS += -Walloc-zero -Walloca -Wstack-protector
CFLAGS += -fanalyzer

CFILES = $(wildcard *.c)
BIN = gem

all:
	$(CC) $(CFLAGS) $(CFILES) -o $(BIN) -lssl -lcrypto

ssl:
	mkdir -p tls
	openssl req -x509 -newkey rsa:4096 -keyout tls/server.key -out tls/server.crt -sha256 -days 3650 -nodes -subj '/CN=localhost'

clean:
	rm -rf $(BIN) tls/
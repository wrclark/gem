SHELL := /usr/bin/bash
CC = gcc
CFLAGS = -std=c89 -O3 -W -Wall -Wextra -pedantic -I.
CFLAGS += -march=native -pipe -D_FORTIFY_SOURCE=2 -fstack-protector-strong
CFLAGS += -fstack-clash-protection
CFLAGS += -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes
CFLAGS += -Wwrite-strings -Wconversion -Wunreachable-code
CFLAGS += -flto -funroll-loops -fPIE -pie -Wl,-z,relro,-z,now
CFLAGS += -fno-strict-aliasing
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
CFLAGS += -Werror -Wlogical-not-parentheses -Wrestrict -Wshadow=local
CFLAGS += -Wunsafe-loop-optimizations -Wstrict-aliasing=3
CFLAGS += -ffunction-sections -fdata-sections -fno-plt
CFLAGS += -Wl,-z,noexecstack -Wl,-z,separate-code
CFLAGS += -fstack-usage

# include if NOT on a Pi
ifneq ($(shell uname -m | grep -E '^arm|^aarch64'), aarch64)
	CFLAGS += -fcf-protection=full
endif

CFILES = $(wildcard *.c)
OBJECTS = $(CFILES:.c=.o)
BIN = gem

all: $(BIN)

$(BIN): $(OBJECTS)
	@echo "cc $(BIN)"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(BIN) -lssl -lcrypto

%.o: %.c
	@echo "cc $<"
	@$(CC) $(CFLAGS) -c $< -o $@

ssl:
	mkdir -p tls
	openssl req -x509 -newkey rsa:4096 -keyout tls/server.key -out tls/server.crt -sha256 -days 3650 -nodes -subj '/CN=localhost'

clean:
	@rm -f $(OBJECTS)
	@rm -f *.su ltrans* *.ltrans.su
SHELL := /usr/bin/bash
CC = gcc
CFLAGS  = -std=c89 -O3 -W -Wall -Wextra -pedantic -I.
CFLAGS += -march=native -pipe -D_FORTIFY_SOURCE=2
CFLAGS += -fstack-protector-strong -fstack-clash-protection
CFLAGS += -flto -funroll-loops -fPIE -pie
CFLAGS += -fno-strict-aliasing -fstack-usage
CFLAGS += -ffunction-sections -fdata-sections -fno-plt
CFLAGS += -Wshadow=global -Wpointer-arith -Wcast-align
CFLAGS += -Wstrict-prototypes -Wwrite-strings -Wconversion
CFLAGS += -Wpedantic -Wsign-conversion
CFLAGS += -Wunreachable-code -Wformat=2
CFLAGS += -Wformat-overflow=2 -Wformat-signedness
CFLAGS += -Wmissing-prototypes -Wmissing-declarations
CFLAGS += -Wredundant-decls -Wstrict-overflow=5
CFLAGS += -Winline -Wlogical-op -Wswitch-enum
CFLAGS += -Wduplicated-cond -Wduplicated-branches
CFLAGS += -Wnull-dereference -Wfloat-equal
CFLAGS += -Wundef -Wbad-function-cast -Wcast-qual
CFLAGS += -Wold-style-definition -Wstrict-aliasing=2
CFLAGS += -Wdouble-promotion -Wimplicit-fallthrough=5
CFLAGS += -Walloc-zero -Walloca -Wstack-protector
CFLAGS += -fanalyzer -Werror
CFLAGS += -Wlogical-not-parentheses -Wrestrict
CFLAGS += -Wunsafe-loop-optimizations -Wstrict-aliasing=3
CFLAGS += -Wl,-z,relro,-z,now
CFLAGS += -Wl,-z,noexecstack -Wl,-z,separate-code

# Include if NOT on a Pi (not ARM nor aarch64)
ifeq ($(shell uname -m | grep -E '^(arm|aarch64)'),)
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
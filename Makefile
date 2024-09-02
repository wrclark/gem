SHELL := /usr/bin/bash
CC = gcc
CFLAGS = -std=c89 -O3 -W -Wall -Wextra -pedantic -I. -march=native -pipe -D_FORTIFY_SOURCE=2 \
                -fstack-protector-strong -fstack-clash-protection -flto -funroll-loops -fPIE \
                -fno-strict-aliasing -fstack-usage -ffunction-sections -fdata-sections \
                -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wwrite-strings \
                -Wconversion -Wpedantic -Wsign-conversion -Wunreachable-code -Wformat=2 \
                -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wstrict-overflow=5 \
                -Winline -Wswitch-enum -Wnull-dereference -Wfloat-equal -Wundef -Wbad-function-cast \
                -Wcast-qual -Wold-style-definition -Wstrict-aliasing=2 -Wdouble-promotion \
                -Wstack-protector -Werror -Wlogical-not-parentheses
CLFAGS += -fanalyzer -Walloca -Walloc-zero -Wformat-overflow=2 -Wformat-signedness \
                      -Wimplicit-fallthrough=5 -Wlogical-op -Wrestrict -Wunsafe-loop-optimizations \
                      -Wshadow=global -Wduplicated-cond -Wduplicated-branches \
                      -Wstrict-aliasing=3
LDFLAGS = -Wl,-z,relro,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code -pie
LIBS = -lssl -lcrypto

# Include if NOT on a Pi (not ARM nor aarch64)
ifeq ($(shell uname -m | grep -E '^(arm|aarch64)'),)
	CFLAGS += -fcf-protection=full
endif

CFILES = $(wildcard *.c)
OBJECTS = $(CFILES:.c=.o)
BIN = gem

all: $(BIN)

$(BIN): $(OBJECTS)
	@echo ">>>> $(BIN)"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(BIN) $(LDFLAGS) $(LIBS)

%.o: %.c
	@echo "cc $<"
	@$(CC) $(CFLAGS) -c $< -o $@

ssl:
	mkdir -p tls
	openssl req -x509 -newkey rsa:4096 -keyout tls/server.key -out tls/server.crt -sha256 -days 3650 -nodes -subj '/CN=localhost'

clean:
	@rm -f $(OBJECTS)
	@rm -f *.su ltrans* *.ltrans.su

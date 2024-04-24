CC=gcc
CFLAGS=-std=c89 -O2 -W -Wall -Wextra -pedantic -I.
CFILES=$(wildcard *.c)
BIN=gem
all:
	$(CC) $(CFLAGS) $(CFILES) -o $(BIN)

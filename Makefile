CC = gcc
CFLAGS = -g -Wall
ROOT=/home/thom/emacs

all: emacs-sqlite.so

%.so: %.o
	$(CC) -shared -o $@ $< -lsqlite3

%.o: %.c
	$(CC) $(CFLAGS) -I$(ROOT)/src -fPIC -c $<

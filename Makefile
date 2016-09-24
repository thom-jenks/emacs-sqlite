CC = g++
CFLAGS = -g -Wall
ROOT=/home/thom/emacs

all: emacs-sql.so

emacs-sql.so: emacs-sql.o sqlite.o
	$(CC) -shared -o $@ $< sqlite.o -lsqlite3

%.o: %.cc
	$(CC) $(CFLAGS) -I$(ROOT)/src -fPIC -c $<

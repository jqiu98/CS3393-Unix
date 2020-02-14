# Makefile
FLAGS = -std=c99 -Wall -O1
all: env

env: env.c
	gcc ${FLAGS} -o $@ $<

clean:
	rm -f env
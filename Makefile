CC=gcc
CFLAGS=-pthread -Wall -Wextra -Werror -O2 -std=c99 -pedantic

makeserver:
	$(CC) $(CFLAGS) -o server main.c

clean:
	rm server

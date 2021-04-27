CC=gcc
CFLAGS=#-std=gnu99 -Wall -Wextra -Werror -pedantic
FILE=main.c

.PHONY: clean all

all: main.c
	$(CC) $(FILE) -o a $(CFLAGS)
	./a 5 5  100 100

clean:
	rm -rf a
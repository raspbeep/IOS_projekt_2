CC=gcc
CFLAGS=#-std=gnu99 -Wall -Wextra -Werror -pedantic
FILE=proj2.c
OBJECTS=proj2

.PHONY: clean all

all: $(FILE)
	$(CC) $(FILE) -o $(OBJECTS) $(CFLAGS)
	./$(OBJECTS) 5 5  100 100

clean:
	rm -rf $(OBJECTS)

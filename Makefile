.Phony: all clean test

all: chash.o chash_example

chash_test: chash_test.c chash.h chash.o
	gcc -Wall -o chash_test chash_test.c chash.o
chash_example: chash_example.c chash.h chash.o

chash.o: chash.c chash.h

test: chash_test
	@./chash_test
	@rm chash_test

clean:
	@rm -rf *.o chash_test chash_example

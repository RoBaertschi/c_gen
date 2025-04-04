CFLAGS += -Wall -Werror -pedantic

ALL: main
	./main

test.c: test.cgen index.ts
	bun run index.ts test.cgen test.c

main: main.c | test.c

CFLAGS := -Wall -Wextra -g

main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o $@ -std=c17 -nostdlib
program.exe: main.o
	gcc main.o -o program.exe -std=c17 -nostdlib

all: program.exe

.DEFAULT_GOAL=all

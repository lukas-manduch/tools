CFLAGS := -Wall -Wextra -g

out/main.o: src/main.c
	$(CC) $(CFLAGS) -c $^ -o $@ -std=c17 -nostdlib
out/program.exe: out/main.o
	gcc $^ -o $@ -std=c17 -nostdlib

all: out/program.exe

clean:
	rm -f ./out/*

.DEFAULT_GOAL=all

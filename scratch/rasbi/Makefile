CFLAGS := -Wall -Wextra -g

out/main.o: src/main.c
	$(CC) $(CFLAGS) -c $^ -o $@ -std=c17 -nostdlib
out/program.exe: out/main.o
	$(CC) $(CFLAGS) $^ -o $@ -std=c17 -nostdlib

all: out/program.exe

clean:
	rm -f ./out/*

out/processed.c: src/main.c
	$(CC) $(CFLAGS) -E $^ -o $@ -std=c17 -nostdlib

.DEFAULT_GOAL=all

CFLAGS := -Wall -Wextra -g -std=c17 -nostdlib 
INCLUDES := -I.
MAIN_SOURCE := $(wildcard src/*tests.c ) src/main.c

out/main.o: $(MAIN_SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) -c src/main.c -o $@ 

out/x86_64.o: src/x86_64.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@ 

out/program.exe: out/main.o out/x86_64.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -std=c17 -nostdlib

all: out/program.exe

clean:
	rm -f ./out/*

out/processed.c: src/main.c
	$(CC) $(CFLAGS) -E $^ -o $@ -std=c17 -nostdlib

.DEFAULT_GOAL=all

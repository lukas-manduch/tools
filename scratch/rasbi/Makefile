CFLAGS := -Wall -Wextra -g
INCLUDES := -I.
SOURCE := $(wildcard src/*.c )

out/main.o: $(SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) -c src/main.c -o $@ -std=c17 -nostdlib 
out/program.exe: out/main.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -std=c17 -nostdlib

all: out/program.exe

clean:
	rm -f ./out/*

out/processed.c: src/main.c
	$(CC) $(CFLAGS) -E $^ -o $@ -std=c17 -nostdlib

.DEFAULT_GOAL=all

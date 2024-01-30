CFLAGS_EXTRA := -Wall -Wextra -g -std=c17 -nostdlib
INCLUDES := -I.
MAIN_SOURCE := $(wildcard src/*tests.c ) src/main.c

out/main.o: $(MAIN_SOURCE)
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -c src/main.c -o $@

out/main.s: $(MAIN_SOURCE)
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -S -fverbose-asm src/main.c -o $@

out/x86_64.o: src/x86_64.c
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -c $^ -o $@

out/program.exe: out/main.o out/x86_64.o
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) $^ -o $@



all: out/program.exe out/main.s

allnotest: CFLAGS_EXTRA+= -DNOTEST
allnotest: clean all


prep:  out/processed.c out/x86_64_prep.c

clean:
	rm -f ./out/*

out/processed.c: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -E $^ -o $@

out/x86_64_prep.c: src/x86_64.c
	$(CC) $(CFLAGS)  $(INCLUDES) -E $^ -o $@

.DEFAULT_GOAL=all
.PHONY=prep all allnotest


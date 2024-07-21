# Optional argument STDLIB=yes STDLIB=no


###### MACOS COMPILATION #######
# On mac, stdlib is used for syscalls and gcc must be installed via homebrew.
# More than this kind of support is now out of the scope
ifdef HOMEBREW_PREFIX
	STDLIB:=$(or $(STDLIB),yes)
	CC=$(HOMEBREW_PREFIX)/bin/gcc
endif
#### END MACOS COMPILATION #####


######## STDLIB CONFIG ##########
# Argument STDLIB controls whether compilation will use (only for syscalls) OS
# stdlib or custom syscall calls defined in source code
STDLIB ?= yes
ifeq ($(STDLIB), no)
	STDLIB_PARAM=-nostdlib -D RASBI_STDLIB=0
	STDLIB_INCLUDE=out/x86_64.o
else
	STDLIB_PARAM= -D RASBI_STDLIB=1
	STDLIB_INCLUDE=out/posix.o
endif
####### END STDLIB CONFIG #######


CFLAGS_EXTRA := -Wall -Wextra -g -std=c17 $(STDLIB_PARAM)
INCLUDES := -I.
MAIN_SOURCE := $(wildcard src/*tests.c ) src/main.c

out/main.o: $(MAIN_SOURCE)
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -c src/main.c -o $@

out/main.s: $(MAIN_SOURCE)
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -S -fverbose-asm src/main.c -o $@

out/x86_64.o: src/x86_64_linux.c
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -c $^ -o $@

out/posix.o: src/posix.c
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) -c $^ -o $@

out/program.exe: out/main.o $(STDLIB_INCLUDE)
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) $(INCLUDES) $^ -o $@

aa:
	echo $(EE)


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


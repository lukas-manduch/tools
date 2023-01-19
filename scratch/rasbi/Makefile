main.o: src/main.c
	gcc -c src/main.c -o main.o -std=c17
program.exe: main.o
	gcc main.o -o program.exe -std=c17

all: program.exe

.DEFAULT_GOAL=all

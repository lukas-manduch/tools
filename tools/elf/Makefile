CFLAGS += -Wall -Wextra -g
#LD = ld

elfer: main.o
	gcc -g -lc  $< -o $@ 

clean:
	rm -f elfer main.o

go_wipe: go_wipe.o
	gcc $(CFLAGS) $< -o $@


all: elfer go_wipe

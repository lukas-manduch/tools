CFLAGS += -Wall -Wextra -g
#LD = ld

build/elfer: build/main.o build/common.o
	gcc $(CFLAGS) build/main.o build/common.o -o $@ 

build/go_pcln: build/go_pcln.o build/common.o
	gcc $(CFLAGS) build/go_pcln.o build/common.o -o $@ 

build/elfdump: build/elfdump.o build/common.o
	gcc $(CFLAGS) build/elfdump.o build/common.o -o $@ 

clean:
	rm build/*.o


build/%.o: src/%.c
	$(CC)  -c $(CFLAGS) $< -o $@

all: build/elfer build/go_pcln build/elfdump

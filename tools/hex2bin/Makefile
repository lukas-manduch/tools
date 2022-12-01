CFLAGS= -fstack-protector-all -fstack-protector-strong -fstack-clash-protection -g  -fcf-protection -O2 -Werror=format-security -Wall -Wl,-z,relro -Wl,-z,now -Wl,-z,defs -fpie -Wl,-pie -fasynchronous-unwind-tables



clean:
	rm -f hex2bin hex2bin.o

hex2bin: hex2bin.o
	$(CC) -o hex2bin $^

test: hex2bin
	bash run_tests.sh

.DEFAULT_GOAL := hex2bin

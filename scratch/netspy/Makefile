# clang  bootstrap.c  -I headers/   -L . -lelf  -lrt -ldl -lpthread -lm -lz -lbpf
# bpftool gen skeleton  bootstrap.bpf.o  > eheader.h
# clang -target bpf -c bootstrap.bpf.c  -I headers/ -g -O2

out/bpf.o:
	clang bpf/bpf.c -target bpf -c  -O2 -o out/bpf.o


out/libbpf.a:
	./libbpf.sh
	cp libbpf/src/build/libbpf.a out/libbpf.a


clean:
	rm -f out/*

all: out/libbpf.a out/bpf.o

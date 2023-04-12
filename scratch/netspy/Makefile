# clang  bootstrap.c  -I headers/   -L . -lelf  -lrt -ldl -lpthread -lm -lz -lbpf
# bpftool gen skeleton  bootstrap.bpf.o  > eheader.h
# clang -target bpf -c bootstrap.bpf.c  -I headers/ -g -O2

out/bpf.o: bpf/bpf.c
	clang bpf/bpf.c -target bpf -c -g  -O2 -o out/bpf.o -Ilibbpf/src/root/usr/include/

out/bpf_header.h: out/bpf.o
	./bpftool gen skeleton out/bpf.o name netspy > out/bpf_header.h

out/libbpf.a:
	./libbpf.sh
	cp libbpf/src/build/libbpf.a out/libbpf.a
clean:
	rm -f out/*


#LIBS := -L out/ -l:libbpf.a -lelf   -lrt -ldl -lpthread -lm -lz  -lelf
LIBS := -L out/ -l:libbpf.a -l:libelf.a   -lrt -ldl -lpthread -lm -l:libz.a  -lelf

out/loader.exe: out/main.o
	gcc out/main.o $(LIBS)  -o $@  

out/main.o: loader.c out/bpf_header.h
	gcc -c loader.c -o out/main.o -Ilibbpf/src/root/usr/include/

all: out/libbpf.a out/bpf_header.h out/bpf.o out/loader.exe

.PHONY=all
.DEFAULT_GOAL=all

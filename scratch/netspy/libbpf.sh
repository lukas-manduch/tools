#!/bin/bash

LIBBPF_GIT="https://github.com/libbpf/libbpf.git"
LIBBPF_TAG="v1.1.0"
set -e

# Set working directory
pushd $(dirname $0) 1>/dev/null
trap 'popd 1>/dev/null' EXIT


if ! [ -e libbpf ] ; then
	git clone "$LIBBPF_GIT" libbpf
fi
pushd .
cd libbpf
git checkout "$LIBBPF_TAG"
cd src
if ! [ -e build ]; then
	mkdir build root
fi
BUILD_STATIC_ONLY=y OBJDIR=build DESTDIR=root make install
popd

bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
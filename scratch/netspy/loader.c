#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/bpf.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <bpf/libbpf.h>
#include "out/bpf_header.h"
#include "shared.h"

#ifndef offsetofend
# define offsetofend(TYPE, FIELD) \
	(offsetof(TYPE, FIELD) + sizeof(((TYPE *)0)->FIELD))
#endif

int bpf(enum bpf_cmd cmd, union bpf_attr *attr, unsigned int size)
{
    return syscall(__NR_bpf, cmd, attr, size);
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
	struct exec_entry* dat = (struct exec_entry*) data;
	printf("%d\t%d\t%s\t", dat->ppid, dat->pid, dat->elfname);
	for (int i = 0; i < dat->argc; i++) {
		printf("%s ", dat->argv[i]);
	}
	printf("\n");
	return 0;
}


int main() {
	union bpf_attr attr;


	// memset(&attr, 0, sizeof(attr));
	// attr.map_type = BPF_MAP_TYPE_RINGBUF;
	// attr.max_entries = 1024*10;
	// attr.key_size = 0;
	// attr.value_size = 8;
	// errno = 0;
	// int bpf_ret = bpf(BPF_MAP_CREATE, &attr, offsetofend(union bpf_attr, btf_vmlinux_value_type_id));
	// if (bpf_ret <= 0) {
	// 	printf("Uh oh, map create failed with %d!\n", bpf_ret);
	// 	perror("Ahoj");
	// 	return 1;
	// }


	printf("Loading something\n");
	libbpf_set_print(libbpf_print_fn);
	struct netspy* skeleton = netspy__open();
	if (netspy__load(skeleton)) {
		printf("Fuckk\n");
		return 1;
	}

	struct ring_buffer *rb =
		ring_buffer__new(bpf_map__fd(skeleton->maps.rb), handle_event, NULL, NULL);
	if (!rb) {
		fprintf(stderr, "Failed to create ring buffer\n");
		return 1;
	}

	int err = netspy__attach(skeleton);
	if (err != 0) {
		printf("Attach error");
		return 1;
	}
	printf("PPID\tPID\tBIN\t\tCALL\n");
	while (1) {
		err = ring_buffer__poll(rb, 100 /* timeout, ms */);
		/* Ctrl-C will cause -EINTR */
		if (err == -EINTR) {
			err = 0;
			break;
		}
		if (err < 0) {
			printf("Error polling perf buffer: %d\n", err);
			break;
		}
	}
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
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
	// Get UTC time
	const int time_buf_size = 60; // yolo
	time_t local_time = time(NULL);
	struct tm* utc_time = gmtime(&local_time);
	char time_buffer[time_buf_size];
	strcpy(time_buffer, "0000-00-00T00:00:00.000");
	if (utc_time) {
		sprintf(time_buffer, "%d-%02d-%02dT%02d:%02d:%02d.000",
				utc_time->tm_year + 1900,
				utc_time->tm_mon,
				utc_time->tm_mday,
				utc_time->tm_hour,
				utc_time->tm_min,
				utc_time->tm_sec);
	}
	time_buffer[time_buf_size-1] = 0;

	// Extract data
	struct exec_entry* dat = (struct exec_entry*) data;

	// Print data
	printf("%s\t%d\t%d\t%s\t", time_buffer, dat->ppid, dat->pid, dat->elfname);
	for (int i = 0; i < dat->argc; i++) {
		printf("%s ", dat->argv[i]);
	}
	printf("\n");
	return 0;
}


int main(int argc, const char** argv) {
	bool debug = false;
	if (argc == 2 && strcmp(argv[1], "--debug") == 0) {
		debug = true;
	}

	libbpf_set_print(NULL);
	if (debug)
		libbpf_set_print(libbpf_print_fn);

	struct netspy* skeleton = netspy__open();
	if (!skeleton || netspy__load(skeleton)) {
		fprintf(stderr, "Cannot load filter. Do you have required permissions?\n");
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
		fprintf(stderr, "Error attaching trace %d\n", err);
		return 1;
	}
	printf("TIME                    PPID\tPID\tBIN\t\tCALL\n");
	while (1) {
		err = ring_buffer__poll(rb, 100 /* timeout, ms */);
		/* Ctrl-C will cause -EINTR */
		if (err == -EINTR) {
			err = 0;
			break;
		}
		if (err < 0) {
			fprintf(stderr, "Error polling perf buffer: %d\n", err);
			break;
		}
	}
	return 0;
}

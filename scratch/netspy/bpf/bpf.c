#include <stddef.h>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_endian.h>
#include "../shared.h"

#ifndef __section
# define __section(NAME)                  \
   __attribute__((section(NAME), used))
#endif


struct bpf_execve {
	char unused[8];
	int syscall_nr;
	const char* filename;
	const char *const *argv;
	const char *const * envp;
};

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, sizeof(struct exec_entry) * 512);
} rb SEC(".maps");

int map_fd = 1;

// TODO: Execveat


SEC("tracepoint/syscalls/sys_enter_execve")
int enter_connect(struct bpf_execve* something) {
	struct exec_entry *entry = 
		bpf_ringbuf_reserve(&rb, sizeof(struct exec_entry), 0);
	if (!entry)
		return 1;
	entry->syscall_nr = something->syscall_nr;
	bpf_probe_read_user_str(entry->elfname, 255, something->filename);
	entry->elfname[255] = 0;
	entry->argc = 0;
	// ARGV
 	for (int i = 0; i < 10; i++) {
 		const char* argvptr = NULL;
 		bpf_probe_read(&argvptr, sizeof(void*), something->argv + i);
 		if (!argvptr) {
 		 	break;
 		}
 		int argvlen =
 			bpf_probe_read_user_str(
 		 			entry->argv[i],
					ARGUMENT_LENGTH,
 					argvptr);
 		if (argvlen <= 0) {
 		 	break;
 		}
		entry->argc = i+1;
 	}
	// My pid
	unsigned long long ptid = bpf_get_current_pid_tgid();
	entry->pid = ptid>>32;
	// Parent pid
	struct task_struct* my_task = (struct task_struct*)bpf_get_current_task();
	int parent_tgid = BPF_CORE_READ(my_task, real_parent, tgid);
	entry->ppid = parent_tgid;
	bpf_ringbuf_submit(entry, 0);
	return map_fd;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";

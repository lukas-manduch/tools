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

struct bpf_execveat {
	char unused[8];
	int syscall_nr;
	char fd[8];
	const char* filename;
	const char *const *argv;
	const char *const *envp;
};

// struct bpf_exec {
// 	const char* filename;
// 	const char *const *argv;
// };

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, sizeof(struct exec_entry) * 512);
} rb SEC(".maps");



void common_setup(struct exec_entry* event) {
	event->elfname[255] = 0;
	event->argc = 0;
	// // My pid
	unsigned long long ptid = bpf_get_current_pid_tgid();
	event->pid = ptid>>32;
	// Parent pid
	struct task_struct* my_task = (struct task_struct*)bpf_get_current_task();
	int parent_tgid = BPF_CORE_READ(my_task, real_parent, tgid);
	event->ppid = parent_tgid;
}

void common_argv(struct exec_entry* event, const char* const* argv) {
 	for (int i = 0; i < 10; i++) {
 		const char* argvptr = NULL;
		bpf_probe_read(&argvptr, sizeof(void*), (argv) + i);
 		if (!argvptr) {
 		 	break;
 		}
 		int argvlen =
 			bpf_probe_read_user_str(
					event->argv[i],
					ARGUMENT_LENGTH,
 					argvptr);
 		if (argvlen <= 0) {
 		 	break;
 		}
		event->argc = i+1;
 	}

}

SEC("tracepoint/syscalls/sys_enter_execveat")
int enter_execveat(struct bpf_execveat* event) {
	struct exec_entry *entry =
		bpf_ringbuf_reserve(&rb, sizeof(struct exec_entry), 0);
	if (!entry)
		return 1;
	bpf_probe_read_user_str(entry->elfname, 255, event->filename);
	common_setup(entry);
	common_argv(entry, event->argv);
	bpf_ringbuf_submit(entry, 0);
	return 0;
}

SEC("tracepoint/syscalls/sys_enter_execve")
int enter_execve(struct bpf_execve* event) {
	struct exec_entry *entry =
		bpf_ringbuf_reserve(&rb, sizeof(struct exec_entry), 0);
	if (!entry)
		return 1;
	bpf_probe_read_user_str(entry->elfname, 255, event->filename);
	common_setup(entry);
	common_argv(entry, event->argv);
	bpf_ringbuf_submit(entry, 0);
	return 0;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";

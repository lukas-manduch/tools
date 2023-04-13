
#define ARGUMENT_LENGTH 256

struct exec_entry {
	char elfname[256];
	char argv[10][ARGUMENT_LENGTH];
	int argc;
	unsigned int pid;
	unsigned int ppid;
};

struct task_struct {
	struct task_struct *real_parent;
	struct task_struct *parent;
	int pid;
	int tgid;
}
#ifndef __GNUC__
 __attribute__((preserve_access_index));
#endif
;

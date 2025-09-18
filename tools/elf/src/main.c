#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include "common.h"

struct Settings {
	const char* filename;
	bool help;
};


static void print_help() {
	printf("Elf parser - ...\n");
	printf("\t-h | --help        \tPrint help and exit.\n");
	printf("\t-f | --file <index>\tElf file. Required\n");
}

static struct Settings parse_arguments(int argc, char** argv) {
	struct Settings settings;
	memset((void*)&settings, 0, sizeof(settings));
	struct option long_options[] = {
		{"help",        0 ,0, 'h'},
		{"file",        1, 0, 'f'},
		{0, 0, 0, 0},
	};
	while (1) {
		int opt;
		int opt_index;
		opt = getopt_long(argc, argv, "+hf:", long_options, &opt_index);
		if (opt == -1) {
			break;
		}
		switch (opt) {
			case 'h':
				settings.help = true;
				break;
			case 'f':
				settings.filename = optarg;
				break;
			default:
				settings.help = true;
				break;
		}
	}
	return settings;
}

void parse_sections(struct ElfMapping* elf);

int main(int argc, char** argv) {
	struct Settings settings = parse_arguments(argc, argv);
	int ret = 0;
	if (settings.help) {
		print_help();
		return 1;
	}
	struct ElfMapping* elf = alloc_elf();
	if ((ret = load_elf(settings.filename, elf, false)) != 0) {
		printf("Error opening file %d\n", -ret);
		return 1;
	}
	//parse_sections(elf);
	
	free_elf(elf);
	return ret;
}


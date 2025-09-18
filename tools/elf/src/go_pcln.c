#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <sys/param.h>
#include <unistd.h>

#include "common.h"

struct Settings {
	const char* filename;
	bool help;
};


static void print_help() {
	printf("go_pcln - manipulate metadata section of GO\n");
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
	if (argc == 1) {
		settings.help = true;
	}
	return settings;
}

int main(int argc, char** argv) {
	struct Settings settings = parse_arguments(argc, argv);
	int ret = 0;
	if (settings.help) {
		print_help();
		return 1;
	}
	struct ElfMapping* elf = alloc_elf();
	if ((ret = load_elf(settings.filename, elf, true)) != 0) {
		printf("Error opening file %d\n", -ret);
		return 1;
	}

	void *start, *end;
	ret = elf_get_section(elf, ".gopclntab", &start, &end);
	if (ret) {
		printf("Problem with finding gopclntab go section\n");
		ret = 1;
		goto exit;
	}
	pretty_print(start, MIN(((char*)end)-((char*)start), 80*10));
	for (size_t i = 0; i < ((char*)end)-((char*)start); i++) {
		char* data = (char*)start;
		if (data[i] == 'm') {
			data[i] = 'n';
		}
	}

exit:
	free_elf(elf);
	return ret;
}


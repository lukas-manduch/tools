#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <fcntl.h>
#include <sys/param.h>
#include <unistd.h>

#include "common.h"

struct Settings {
	const char* filename;
	const char* directory;
	bool help;
};


static void print_help() {
	printf("elfdump - Dump all sections and metadata of elf file to folder\n");
	printf("\t-h | --help        \tPrint help and exit.\n");
	printf("\t-d | --directory   \tDump all data to this directory. Must exist and must be empty\n");
	printf("\t-f | --file <index>\tElf file. Required\n");
}

static struct Settings parse_arguments(int argc, char** argv) {
	struct Settings settings;
	memset((void*)&settings, 0, sizeof(settings));
	struct option long_options[] = {
		{"help",        0 ,0, 'h'},
		{"directory",   0 ,0, 'd'},
		{"file",        1, 0, 'f'},
		{0, 0, 0, 0},
	};
	while (1) {
		int opt;
		int opt_index;
		opt = getopt_long(argc, argv, "+hf:d:", long_options, &opt_index);
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
			case 'd':
				settings.directory = optarg;
				break;
			default:
				settings.help = true;
				break;
		}
	}
	if (argc == 1 || !settings.filename || !settings.directory) {
		settings.help = true;
	}
	return settings;
}

/** Return 0 on folder ok and non 0 on folder unaceptable
 */
static int prepare_directory(const char* directory_path) {
	int ret = 0;
	DIR* pdir = opendir(directory_path);
	if (!pdir) {
		ret = 1;
		return ret;
	}
	struct dirent* entry = readdir(pdir);
	while ((entry = readdir(pdir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0) {
			continue;
		}
		if (strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		// There is some unexpected file. Exit
		ret = 2;
		break;

	}
	closedir(pdir);
	return ret;
}

static int open_section_file(const char* directory, const char* filename) {
	int ret = 0;
	int filefd = -1;
	int dirfd = open(directory, O_DIRECTORY, 0);
	if (dirfd < 0) {
		ret = -1;
		goto exit;
	}
	filefd = openat(dirfd, filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (filefd < 0) {
		ret = -1;
		printf("Cannot create file %s\n", filename);
		goto exit;
	}
	ret = filefd;

exit:
	if (dirfd >= 0) {
		close(dirfd);
	}
	return ret;
}

int main(int argc, char** argv) {
	struct Settings settings = parse_arguments(argc, argv);
	int ret = 0;
	if (settings.help) {
		print_help();
		return 1;
	}
	struct ElfMapping* elf = alloc_elf();
	if (prepare_directory(settings.directory)) {
		printf("Directory must exist and be empty\n");
		ret = 1;
		goto exit;
	}
	if ((ret = load_elf(settings.filename, elf, false)) != 0) {
		printf("Error opening file %d\n", -ret);
		return 1;
	}
	// For each section
	for (int i = 0; i < elf->elf_header->e_shnum; i++) {
		const char* section_name = get_section_name(elf, i);
		if (!section_name) {
			printf("Section %d missing section name.\n", i);
		}
		void *start, *end;
		char filename[100];
		if (elf_get_section_by_index(elf, i, &start, &end) != 0) {
			printf("Cannot retrieve section %d\n", i);
			continue;
		}
		snprintf(filename, 100, "%02d%s", i, section_name ? section_name : "");
		int section_file_fd = open_section_file(settings.directory, filename);
		if (section_file_fd < 0) {
			printf("Cannot open file for section %d\n", section_file_fd);
			continue;
		}
		size_t section_length = ((char*)end)-((char*)start);
		long written = write(section_file_fd, start, section_length);
		printf("Wrote %lu %ld\n", section_length, written);
		if (written == -1) {
			perror("OOO");
		}
		close(section_file_fd);
	}
exit:
	if (elf) {
		free_elf(elf);
		elf = 0;
	}
	return ret;
}


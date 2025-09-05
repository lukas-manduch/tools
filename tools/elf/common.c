#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

struct ElfMapping* alloc_elf() {
	struct ElfMapping* ptr=
		(struct ElfMapping*)malloc(sizeof(struct ElfMapping));
	if (!ptr) {
		printf("Malloc failed\n");
		exit(1);
	}
	memset((void*)ptr, 0, sizeof(*ptr));
	return ptr;
}

int load_elf(const char* filename, struct ElfMapping* mapping) {
	int ret = 0;
	int fd = open(filename,O_RDONLY, 0);
	struct stat stats;
	if (fd < 0) {
		ret = -errno;
		goto error;
	}

	ret = fstat(fd, &stats);
	if (ret < 0) {
		ret = -errno;
		goto error;
	}
	mapping->length = stats.st_size;
	mapping->start = mmap(NULL, mapping->length ,PROT_READ , MAP_PRIVATE, fd, 0);
	if (mapping->start == MAP_FAILED) {
		mapping->start = 0;
		ret = -errno;
		close(fd);
		goto error;
	}
	close(fd);
	ret = 0;
	return ret;
error:
	if (ret < 0) {
		return ret;
	}
	return -1;
}

void free_elf(struct ElfMapping* mapping) {
	munmap(mapping->start, mapping->length);
	mapping->start = NULL;
	mapping->elf_header = 0;
}


Elf64_Shdr* _get_sstrtab(struct ElfMapping* elf) {
	Elf64_Ehdr *hdr = elf->elf_header;
	Elf64_Shdr* section = (Elf64_Shdr*)(((char*)elf->start) + hdr->e_shoff);
	Elf64_Shdr* last_viable = NULL;
	for (int i = 0; i < hdr->e_shnum; i++, section++) {
		if (section->sh_type == SHT_STRTAB) {
			last_viable = section;
		}
	}
	return last_viable;
}

/** This only prints small part of section and only printable characters
 */
void pprint_section(struct ElfMapping* elf, Elf64_Shdr* section) {
	char* data = (char*)elf->start + section->sh_offset;
	size_t total_printed = 0;
	for (size_t i = 0; i < section->sh_size; i++) {
		if (!isprint(data[i]))
			continue;
		total_printed++;
		if (isspace(data[i])) {
			putchar(' ');
		} else {
			putchar(data[i]);
		}
		if (total_printed >= 80*7) {
			return;
		}
		if (total_printed % 80 == 0) {
			putchar('\n');
		}
	}
	putchar('\n');
}

void parse_sections(struct ElfMapping* elf) {
	Elf64_Shdr *strtab = _get_sstrtab(elf);
	//Elf64_Shdr *strtab = (Elf64_Shdr*)(((char*)elf->start) + elf->elf_header->e_shstrndx);
	if (memcmp(ELFMAG, &(elf->elf_header->e_ident[0]), 4) != 0) {
		printf("Not an elf\n");
		return;
	}
	printf("Size of ehdr %lu and real size %u\n", sizeof(Elf64_Ehdr), elf->elf_header->e_ehsize);
	printf("Entry point 0x%lx == %lu\n", elf->elf_header->e_entry, elf->elf_header->e_entry);
	printf("Section offset %lu, program offset %lu\n", elf->elf_header->e_shoff, elf->elf_header->e_phoff);
	printf("Section header entry size %d, expected %ld\n", elf->elf_header->e_shentsize, sizeof(Elf64_Shdr));
	printf("Index of section header string table %d\n", elf->elf_header->e_shstrndx);
	printf("Sections:\n");
	Elf64_Shdr *section = (Elf64_Shdr*)(((char*)elf->start) + elf->elf_header->e_shoff);
	for (int i = 0; i < elf->elf_header->e_shnum; i++, section++) {
		if (strtab) {
			printf("Name %d", section->sh_name);
			char* strtab_data = ((char*)elf->start) + strtab->sh_offset;
			printf(" %s\n", &strtab_data[section->sh_name]);
		}
		printf("[%d]Offset %ld size %lx\n", i, section->sh_offset, section->sh_size);
		printf("    Virtual addr %lu Table size %lu align %lu  ", section->sh_addr, section->sh_entsize, section->sh_addralign);
		if (section->sh_type == SHT_STRTAB) {
			printf("STRTAB");
		}
		if (section->sh_type == SHT_SYMTAB) {
			printf("SYMTAB");
		}
		if (section->sh_type == SHT_RELA) {
			printf("RELA");
		}
		if (section->sh_type == SHT_REL) {
			printf("REL");
		}
		if (section->sh_type == SHT_RELR) {
			printf("RELR");
		}
		printf("\n");
	}
}

void parse_segments(struct ElfMapping* elf) {
	Elf64_Phdr* segment = (Elf64_Phdr*)(((char*)elf->start) + elf->elf_header->e_phoff);

	for (int i = 0; i < elf->elf_header->e_phnum; i++, segment++) {
		switch(segment->p_type) {
			case PT_LOAD:
				printf("PT_LOAD");
				break;
			case PT_DYNAMIC:
				printf("PT_DYNAMIC");
				break;
			case PT_INTERP:
				printf("PT_INTERP");
				break;
			case PT_NOTE:
				printf("PT_NOTE");
				break;
			case PT_SHLIB:
				printf("PT_SHLIB");
				break;
			case PT_PHDR:
				printf("PT_PHDR");
				break;
			case PT_TLS:
				printf("PT_TLS");
				break;
			case PT_NUM:
				printf("PT_NUM");
				break;
			case PT_GNU_EH_FRAME:
				printf("PT_GNU_EH_FRAME");
				break;
			case PT_GNU_STACK:
				printf("PT_GNU_STACK");
				break;
			case PT_GNU_RELRO:
				printf("PT_GNU_RELRO");
				break;
			case PT_GNU_PROPERTY:
				printf("PT_GNU_PROPERTY");
				break;
			default:
				printf("PT Unknown 0x%x", segment->p_type);
				break;
		}
		printf("\n");
	}
}

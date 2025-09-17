#include <ctype.h>
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

int load_elf(const char* filename, struct ElfMapping* mapping, bool write) {
	int ret = 0;
	int mode = write ? O_RDWR : O_RDONLY;
	int fd = open(filename,mode, 0);
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
	int prot = write ? PROT_READ | PROT_WRITE : PROT_READ;
	mapping->start = mmap(NULL, mapping->length , prot, MAP_SHARED, fd, 0);
	if (mapping->start == MAP_FAILED) {
		mapping->start = 0;
		ret = -errno;
		close(fd);
		goto error;
	}
	close(fd);
	ret = 0;
	mapping->elf_header = (Elf64_Ehdr*)mapping->start;
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

void pretty_print(void* data, unsigned short length) {
	const char* cdata = (const char*)data;
	for (size_t i = 0; i < length; i++) {
		if (isspace(cdata[i])) {
			putchar(' ');
		}
		else if (!isprint(cdata[i])) {
			putchar('?');
		} else {
			putchar(cdata[i]);
		}
		if (i % 80 == 0) {
			putchar('\n');
		}
		if (i >= 80*20000) {
			return;
		}
	}
	putchar('\n');
}

static Elf64_Shdr* _get_sstrtab(struct ElfMapping* elf) {
	Elf64_Ehdr *hdr = elf->elf_header;
	Elf64_Shdr* section = (Elf64_Shdr*)(((char*)elf->start) + hdr->e_shoff);
	return &section[elf->elf_header->e_shstrndx];
	//Elf64_Shdr* last_viable = NULL;
	//for (int i = 0; i < hdr->e_shnum; i++, section++) {
	//	if (section->sh_type == SHT_STRTAB) {
	//		last_viable = section;
	//	}
	//}
	//return last_viable;
}

const char* index_strtab(struct ElfMapping* elf, unsigned int index) {
	Elf64_Shdr* strtab = _get_sstrtab(elf);
	if (!strtab) {
		return NULL;
	}
	const char* data = ((const char*)elf->start) + strtab->sh_offset;
	return &data[index];
}


/** This only prints small part of section and only printable characters
 */
void pprint_section(struct ElfMapping* elf, Elf64_Shdr* section) {
	char* data = (char*)elf->start + section->sh_offset;
	size_t total_printed = 0;
	for (size_t i = 0; i < section->sh_size; i++) {
		total_printed++;
		if (isspace(data[i])) {
			putchar(' ');
		}
		else if (!isprint(data[i])) {
			putchar('?');
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

Elf64_Shdr* elf_get_section_raw(struct ElfMapping* elf, const char* name) {
	Elf64_Ehdr *hdr = elf->elf_header;
	Elf64_Shdr* section = (Elf64_Shdr*)(((char*)elf->start) + hdr->e_shoff);
	for (int i = 0; i < hdr->e_shnum; i++, section++) {
		const char* current_name = index_strtab(elf, section->sh_name);
		if (!current_name) {
			return NULL;
		}
		if (strcmp(current_name, name) == 0) {
			return section;
		}
	}
	return NULL;;
}

int elf_get_section(struct ElfMapping* elf, const char* name, void** start, void** end) {
	if (!start || !end || !elf) {
		return -1;
	}
	Elf64_Shdr* section = elf_get_section_raw(elf, name);
	if (!section) {
		return -1;
	}
	*start = ((char*)elf->start) + section->sh_offset;
	*end = ((char*)*start) + section->sh_size;
	return 0;
}

int elf_get_section_by_index(struct ElfMapping* elf, unsigned short index, void** start, void** end) {
	if (!start || !end || !elf) {
		return -1;
	}
	Elf64_Ehdr *hdr = elf->elf_header;
	Elf64_Shdr* section = (Elf64_Shdr*)(((char*)elf->start) + hdr->e_shoff);
	section = section + index;
	*start = ((char*)elf->start) + section->sh_offset;
	*end = ((char*)*start) + section->sh_size;
	return 0;
}

const char* get_section_name(struct ElfMapping* elf, int index) {
	Elf64_Ehdr *hdr = elf->elf_header;
	Elf64_Shdr* section = (Elf64_Shdr*)(((char*)elf->start) + hdr->e_shoff);
	return index_strtab(elf, section[index].sh_name);
}

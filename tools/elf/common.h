#ifndef COMMON_H
#define COMMON_H
#include <elf.h>
#include <stdint.h>
struct ElfMapping {
	void *start;
	size_t length;
	Elf64_Ehdr* elf_header;
	Elf64_Shdr* shdr; // Section headers
};
struct ElfMapping* alloc_elf();
int load_elf(const char* filename, struct ElfMapping* mapping);
void free_elf(struct ElfMapping* mapping);
void find_section(struct ElfMapping* elf, const char* secion_name);
#endif

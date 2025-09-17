#ifndef COMMON_H
#define COMMON_H
#include <elf.h>
#include <stdbool.h>
#include <stdint.h>
struct ElfMapping {
	void *start;
	size_t length;
	Elf64_Ehdr* elf_header;
};
struct ElfMapping* alloc_elf();
int load_elf(const char* filename, struct ElfMapping* mapping, bool write);
void free_elf(struct ElfMapping* mapping);

/** In open elf mapping finds section and returns pointer to it's metadata.
 * This points inside the ElfMapping and should not be freed separately.
 */
Elf64_Shdr* elf_get_section_raw(struct ElfMapping*, const char* name);

/** In open elf file find section and set START to address of the section. END
 * points one past last addressable byte of section
 *
 * Returns 0 on success and non zero on error
 */
int elf_get_section(struct ElfMapping*, const char* name, void** start, void** end);

/** Given section index, retrieve it's start and end address.
 *
 * Returns 0 on success
 */
int elf_get_section_by_index(struct ElfMapping* elf, unsigned short index, void** start, void** end);
/** Pretty prints up to length bytes
 */
void pretty_print(void* data, unsigned short length);
/** Given section index, returns section name or NULL
 */
const char* get_section_name(struct ElfMapping* elf, int index);
#endif

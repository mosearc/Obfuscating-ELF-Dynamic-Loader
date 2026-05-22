#ifndef RELOCATE_H
#define RELOCATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/my_ph.h"
#include "../include/my_elfh.h"

#include <sys/mman.h>

typedef struct My_Phdr My_Phdr;

#define DT_RELA 7    /* Address of Rela relocs */
#define DT_RELASZ 8  /* Total size of Rela relocs */
#define DT_RELAENT 9 /* Size of one Rela reloc */
#define DT_NULL 0    /* Marks end of dynamic section */
#define ELF64_R_TYPE(i) ((uint32_t)(i) & 0xffffffff)
#define R_X86_64_RELATIVE 8     /* Adjust by program base */
#define R_AARCH64_RELATIVE 1027 /* Adjust by program base.  */
#define DT_RELACOUNT 0x6ffffff9

int relocate(const My_Ehdr *ehdr, const My_Phdr *phdr, const char *lib, const uint64_t base_address, const long page_size);

#endif
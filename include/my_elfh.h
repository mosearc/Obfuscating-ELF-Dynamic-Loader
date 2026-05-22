#ifndef MY_ELFH_H
#define MY_ELFH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#define EI_MAG0 0  // 0x7F
#define EI_MAG1 1  // 'E'
#define EI_MAG2 2  // 'L'
#define EI_MAG3 3  // 'F'
#define EI_CLASS 4 // Architecture (32/64)

#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

#define ELFCLASS64 2 // 64-bit Architecture

#define ET_DYN 3 // Shared Object

typedef struct My_Ehdr My_Ehdr;

bool elf_check_all(const char *lib, My_Ehdr *hdr);
My_Ehdr *construct_ehdr(void);
void destruct_ehdr(My_Ehdr *hdr);
uint16_t get_ephsize(const My_Ehdr *hdr);
uint16_t get_ephnum(const My_Ehdr *hdr);
uint64_t get_phoffest(const My_Ehdr *hdr);
uint64_t get_eentry(const My_Ehdr *hdr);

#endif
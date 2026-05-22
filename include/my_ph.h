#ifndef MY_PH_H
#define MY_PH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/my_elfh.h"
#include "../include/mmapp.h"
#include "../include/relocate.h"

#define PT_LOAD 1 /* Loadable program segment */
#define PT_DYNAMIC 2

typedef struct My_Phdr My_Phdr;

bool p_check_all(My_Phdr *pdr, const My_Ehdr *hdr, const char *lib);
My_Phdr *construct_phdr(void);
void destruct_phdr(My_Phdr *pdr);
uint64_t get_p_tot_mem(My_Phdr *pdr, const My_Ehdr *hdr, const char *lib);

uint64_t get_pmemsz(const My_Phdr *phdr);
uint64_t get_pvaddr(const My_Phdr *phdr);
uint32_t get_pflag(const My_Phdr *phdr);
uint32_t get_ptype(const My_Phdr *phdr);
uint64_t get_pfilesz(const My_Phdr *phdr);
uint64_t get_ppoffset(const My_Phdr *phdr);
uint64_t get_pfirstv();
uint64_t get_mmap_totmem();

#include <sys/mman.h>
#include <fcntl.h>

#define PF_X 1 /* Segment is executable */
#define PF_W 2 /* Segment is writable */
#define PF_R 4 /* Segment is readable */

void *load_in_mem(const My_Ehdr *ehdr, My_Phdr *phdr, const char *lib);

#endif
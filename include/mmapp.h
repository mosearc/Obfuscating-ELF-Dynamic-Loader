#ifndef MMAPP_H
#define MMAPP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/my_ph.h"
#include "../include/my_elfh.h"

#include <sys/mman.h>
#include <fcntl.h>

typedef struct My_Phdr My_Phdr;

#define PF_X 1 /* Segment is executable */
#define PF_W 2 /* Segment is writable */
#define PF_R 4 /* Segment is readable */

uint64_t memory_reserving(const uint64_t total_mem);
int segment_loading(const My_Phdr *phdr, const int fd, const uint64_t base_address, const long page_size);
int segment_protceting(const My_Phdr *phdr, const uint64_t base_address, const long page_size);

#endif
#include "../include/my_ph.h"

/* IN THIS FILE:
 *====================================
 * CHALLENGE 3
 * CHALLENGE 4 CALLER
 * CHALLENGE 5 CALLER
 *====================================
 */

struct My_Phdr
{
    uint32_t p_type;   /* Segment type */
    uint32_t p_flags;  /* Segment flags */
    uint64_t p_offset; /* Segment file offset */
    uint64_t p_vaddr;  /* Segment virtual address */
    uint64_t p_paddr;  /* Segment physical address */
    uint64_t p_filesz; /* Segment size in file */
    uint64_t p_memsz;  /* Segment size in memory */
    uint64_t p_align;  /* Segment alignment */
};

// shared state informations
static uint64_t first_vaddr = UINT64_MAX;
static uint64_t total_mmap_size = 0;

// Check the size declared in e_phentsize that must match the size of the struct.
__attribute__((nonnull(1, 2))) static bool p_check_size(const My_Ehdr *hdr, const My_Phdr *pdr)
{

    if (get_ephsize(hdr) != sizeof(*pdr))
    {

        write(2, "Program Headers - Elf Header size Mismatch\n", strlen("Program Headers - Elf Header size Mismatch\n"));
        return false;
    }

    return true;
}

bool p_check_all(My_Phdr *pdr, const My_Ehdr *hdr, const char *lib)
{

    if (!pdr || !hdr)
    {

        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        return false;
    }

    //check0 SIZE
    if (!p_check_size(hdr, pdr))
        return false;

    FILE *elf_file = fopen(lib, "rb");
    if (!elf_file)
    {

        write(2, "Failed to open file\n", strlen("Failed to open file\n"));
        return false;
    }

    // Jump to the start of the program header table.
    if (fseek(elf_file, get_phoffest(hdr), SEEK_SET) != 0)
    {

        write(2, "Failed to seek to program header table\n", strlen("Failed to seek to program header table\n"));
        fclose(elf_file);
        elf_file = NULL;
        return false;
    }

    int load_count = 0;
    uint64_t p_addr = 0; // p_vaddr of the previous PT_LOAD
    uint64_t p_end = 0;  // p_vaddr + p_memsz of the previous PT_LOAD

    for (int i = 0; i < get_ephnum(hdr); i++)
    {

        if (fread(pdr, get_ephsize(hdr), 1, elf_file) != 1)
        {

            write(2, "Failed to read a PROGRAM header\n", strlen("Failed to read a PROGRAM header\n"));
            fclose(elf_file);
            elf_file = NULL;
            return false;
        }

        if (pdr->p_type != PT_LOAD)
        {
            continue;
        }

        load_count++;

        if (load_count == 1)
        {

            // end of segment headers
            uint64_t phtbl_end = get_phoffest(hdr) + (uint64_t)get_ephnum(hdr) * (uint64_t)get_ephsize(hdr);

// check1 if the first load segment spans over all segment headers
            if (pdr->p_offset > get_phoffest(hdr) || pdr->p_offset + pdr->p_filesz < phtbl_end)
            {
                write(2, "First load segment does NOT spans over all segment headers.\n", strlen("First load segment does NOT spans over all segment headers.\n"));
                fclose(elf_file);
                elf_file = NULL;
                return false;
            }

            first_vaddr = pdr->p_vaddr;
        }

// check2 if he PT_LOAD segments are in ascending order of p_vaddr.
        if (load_count > 1 && p_addr > pdr->p_vaddr)
        {

            write(2, "PT_LOAD segments are NOT in ascending order of p_vaddr\n", strlen("PT_LOAD segments are NOT in ascending order of p_vaddr\n"));
            fclose(elf_file);
            elf_file = NULL;
            return false;
        }

// check3 if The PT_LOAD segments do overlap.
        if (load_count > 1 && p_end > pdr->p_vaddr)
        {
            write(2, "Overlap\n", strlen("Overlap\n"));
            fclose(elf_file);
            elf_file = NULL;
            return false;
        }

        p_addr = pdr->p_vaddr;
        p_end = pdr->p_vaddr + pdr->p_memsz;
    }

    if (!load_count)
    {
        write(2, "DL library has ZERO load segment\n", strlen("DL library has ZERO load segment\n"));
        fclose(elf_file);
        elf_file = NULL;
        return false;
    }

    fclose(elf_file);
    elf_file = NULL;
    return true;
}

// Compute the total memory size between the first PT_LOAD and the end of the last PT_LOAD
// It is the range of memory that needs to be reserved
uint64_t
get_p_tot_mem(My_Phdr *pdr, const My_Ehdr *hdr, const char *lib)
{

    if (!pdr || !hdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        return 0;
    }

    FILE *elf_file = fopen(lib, "rb");
    if (!elf_file)
    {
        write(2, "Failed to open file\n", strlen("Failed to open file\n"));
        return 0;
    }

    if (fseek(elf_file, get_phoffest(hdr), SEEK_SET) != 0)
    {
        write(2, "Failed to seek to program header table\n", strlen("Failed to seek to program header table\n"));
        fclose(elf_file);
        elf_file = NULL;
        return 0;
    }

    int load_count = 0;
    uint64_t start = 0;
    uint64_t end = 0;

    for (int i = 0; i < get_ephnum(hdr); i++)
    {

        if (fread(pdr, get_ephsize(hdr), 1, elf_file) != 1)
        {
            write(2, "Failed to read a PROGRAM header\n", strlen("Failed to read a PROGRAM header\n"));
            fclose(elf_file);
            elf_file = NULL;
            return 0;
        }

        if (pdr->p_type != PT_LOAD)
        {
            continue;
        }

        load_count++;

        if (load_count == 1)
        {
            start = pdr->p_vaddr;
            first_vaddr = pdr->p_vaddr;
        }
        end = pdr->p_vaddr + pdr->p_memsz;
    }

    fclose(elf_file);
    elf_file = NULL;

    if (end <= start)
    {
        write(2, "Invalid segment memory range\n", strlen("Invalid segment memory range\n"));
        return 0;
    }

    return end - start;
}

// Top level loader for loading and patching the relocation and return the base address
void *load_in_mem(const My_Ehdr *ehdr, My_Phdr *phdr, const char *lib)
{

    if (!phdr || !ehdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        return NULL;
    }

    // system page size -- needed for alignment computation
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 0)
    {
        write(2, "Problems in calculation of the page size\n", strlen("Problems in calculation of the page size\n"));
        return NULL;
    }

    uint64_t total_mem = get_p_tot_mem(phdr, ehdr, lib);
    if (total_mem == 0)
    {
        write(2, "Invalid total memory size\n", strlen("Invalid total memory size\n"));
        return NULL;
    }

    // Round up to a multiple of the page size to be eventually passed to mmap adn munmap
    total_mmap_size = ((total_mem + page_size - 1) / page_size) * page_size;

    // reserve a contiguous memory region and get the base address
    uint64_t base_address = memory_reserving(total_mem);
    if (base_address == UINT64_MAX)
    {

        write(2, "memory reservation error\n", strlen("memory reservation error\n"));
        return NULL;
    }

    FILE *elf_file = fopen(lib, "rb");
    if (!elf_file)
    {
        write(2, "Failed to open file\n", strlen("Failed to open file\n"));
        munmap((void *)(base_address + first_vaddr), total_mmap_size);
        return NULL;
    }

    if (fseek(elf_file, get_phoffest(ehdr), SEEK_SET) != 0)
    {
        write(2, "Failed to seek to program header table\n", strlen("Failed to seek to program header table\n"));
        fclose(elf_file);
        elf_file = NULL;
        munmap((void *)(base_address + first_vaddr), total_mmap_size);
        return NULL;
    }

    // for mmap that requires a real file descriptor, not FILE*
    int fd = open(lib, O_RDONLY);
    if (fd == -1)
    {
        fclose(elf_file);
        munmap((void *)(base_address + first_vaddr), total_mmap_size);
        return NULL;
    }

    for (int i = 0; i < get_ephnum(ehdr); i++)
    {

        if (fread(phdr, get_ephsize(ehdr), 1, elf_file) != 1)
        {
            write(2, "Failed to read a PROGRAM header\n", strlen("Failed to read a PROGRAM header\n"));
            fclose(elf_file);
            close(fd);
            munmap((void *)(base_address + first_vaddr), total_mmap_size);
            elf_file = NULL;
            return NULL;
        }

        if (phdr->p_type != PT_LOAD)
        {
            continue;
        }

        // memory loading of the segments
        int segment_loaded = segment_loading(phdr, fd, base_address, page_size);

        if (!segment_loaded)
        {
            write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
            fclose(elf_file);
            close(fd);
            munmap((void *)(base_address + first_vaddr), total_mmap_size);
            return NULL;
        }

        // apply the appropriate permission
        int segment_protected = segment_protceting(phdr, base_address, page_size);

        if (!segment_protected)
        {
            write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
            fclose(elf_file);
            close(fd);
            munmap((void *)(base_address + first_vaddr), total_mmap_size);
            return NULL;
        }
    }

    if (fseek(elf_file, get_phoffest(ehdr), SEEK_SET) != 0)
    {
        write(2, "Failed to seek to program header table\n", strlen("Failed to seek to program header table\n"));
        fclose(elf_file);
        elf_file = NULL;
        close(fd);
        munmap((void *)(base_address + first_vaddr), total_mmap_size);
        return NULL;
    }

    for (int i = 0; i < get_ephnum(ehdr); i++)
    {

        if (fread(phdr, get_ephsize(ehdr), 1, elf_file) != 1)
        {
            write(2, "Failed to read a PROGRAM header\n", strlen("Failed to read a PROGRAM header\n"));
            fclose(elf_file);
            elf_file = NULL;
            close(fd);
            munmap((void *)(base_address + first_vaddr), total_mmap_size);
            return NULL;
        }

        if (phdr->p_type == PT_DYNAMIC)
        {
            // dot the relocation
            int rel = relocate(ehdr, phdr, lib, base_address, page_size);
            if (!rel)
            {
                write(2, "Failure in relocation\n", strlen("Failure in relocation\n"));
                munmap((void *)(base_address + first_vaddr), total_mmap_size);
                fclose(elf_file);
                elf_file = NULL;
                close(fd);
                return NULL;
            }
        }
    }

    fclose(elf_file);
    close(fd);

    // The base address (NOT the mapping pointer) that is the loader's handle.
    return (void *)base_address;
}

// phdr constructor
__attribute__((malloc))
My_Phdr *
construct_phdr(void)
{
    return malloc(sizeof(struct My_Phdr));
}

// phdr destructor
void destruct_phdr(My_Phdr *pdr)
{

    if (!pdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    free(pdr);
    pdr = NULL;
}

//***
// GETTERS

uint64_t get_pmemsz(const My_Phdr *phdr)
{

    if (!phdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    return phdr->p_memsz;
}

uint64_t get_pvaddr(const My_Phdr *phdr)
{

    if (!phdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    return phdr->p_vaddr;
}

uint32_t get_pflag(const My_Phdr *phdr)
{

    if (!phdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    return phdr->p_flags;
}

uint32_t get_ptype(const My_Phdr *phdr)
{

    if (!phdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    return phdr->p_type;
}

uint64_t get_pfilesz(const My_Phdr *phdr)
{

    if (!phdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    return phdr->p_filesz;
}

uint64_t get_pfirstv()
{

    if (first_vaddr == UINT64_MAX)
    {
        write(2, "first vaddr is zero\n", strlen("first vaddr is zero\n"));
        exit(EXIT_FAILURE);
    }

    return first_vaddr;
}

uint64_t get_ppoffset(const My_Phdr *phdr)
{

    if (!phdr)
    {
        write(2, "you might have passed NULL ProgramH structure\n", strlen("you might have passed NULL ProgramH structure\n"));
        exit(EXIT_FAILURE);
    }

    return phdr->p_offset;
}

// api to get the memory for the cleanup in othe modules
uint64_t get_mmap_totmem()
{

    if (total_mmap_size == 0)
    {
        write(2, "you should reserve memory w mmap before\n", strlen("you should reserve memory w mmap before\n"));
        exit(EXIT_FAILURE);
    }

    return total_mmap_size;
}

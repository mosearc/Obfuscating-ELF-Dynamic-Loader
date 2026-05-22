#include "../include/my_elfh.h"

/* IN THIS FILE:
 *====================================
 * CHALLENGE 2
 *====================================
 */

struct My_Ehdr
{
    unsigned char e_ident[16]; /* Magic number and other info */
    uint16_t e_type;           /* Object file type */
    uint16_t e_machine;        /* Architecture */
    uint32_t e_version;        /* Object file version */
    uint64_t e_entry;          /* Entry point virtual address */
    uint64_t e_phoff;          /* Program header table file offset */
    uint64_t e_shoff;          /* Section header table file offset */
    uint32_t e_flags;          /* Processor-specific flags */
    uint16_t e_ehsize;         /* ELF header size in bytes */
    uint16_t e_phentsize;      /* Program header table entry size */
    uint16_t e_phnum;          /* Program header table entry count */
    uint16_t e_shentsize;      /* Section header table entry size */
    uint16_t e_shnum;          /* Section header table entry count */
    uint16_t e_shstrndx;       /* Section header string table index */
};

// check 1: binary is of format elf
__attribute__((nonnull)) static bool elf_check_magic(const My_Ehdr *hdr)
{

    if (hdr->e_ident[EI_MAG0] != ELFMAG0)
    {
        write(2, "Magic Number 1 incorrect.\n", strlen("Magic Number 1 incorrect.\n"));
        return false;
    }
    if (hdr->e_ident[EI_MAG1] != ELFMAG1)
    {
        write(2, "Magic Number 2 incorrect.\n", strlen("Magic Number 2 incorrect.\n"));
        return false;
    }
    if (hdr->e_ident[EI_MAG2] != ELFMAG2)
    {

        write(2, "Magic Number 3 incorrect.\n", strlen("Magic Number 3 incorrect.\n"));
        return false;
    }
    if (hdr->e_ident[EI_MAG3] != ELFMAG3)
    {

        write(2, "Magic Number 4 incorrect.\n", strlen("Magic Number 4 incorrect.\n"));
        return false;
    }
    return true;
}

// check 2: binary is of architecture 64-bit
__attribute__((nonnull)) static bool elf_check_arch(const My_Ehdr *hdr)
{

    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    {

        write(2, "Unsupported Architecture (!64)\n", strlen("Unsupported Architecture (!64)\n"));
        return false;
    }
    return true;
}

// check 3: binary is of type dynamic
__attribute__((nonnull)) static bool elf_check_type(const My_Ehdr *hdr)
{

    if (hdr->e_type != ET_DYN)
    {

        write(2, "Unwanted Type (!Dynamic)\n", strlen("Unwanted Type (!Dynamic)\n"));
        return false;
    }
    return true;
}

// check 4: the size of my ehdr structure is equal to the field ehsize
__attribute__((nonnull)) static bool elf_check_size(const My_Ehdr *hdr)
{

    if (hdr->e_ehsize != sizeof(*hdr))
    {

        write(2, "Size Error\n", strlen("Size Error\n"));
        return false;
    }

    return true;
}

// the number of segments (headers) is strictly greater than zero.
__attribute__((nonnull)) static bool elf_check_segments(const My_Ehdr *hdr)
{

    if (hdr->e_phnum <= 0)
    {

        write(2, "Sections Less Than 1\n", strlen("Sections Less Than 1\n"));
        return false;
    }
    return true;
}

// constructor
__attribute__((malloc))
My_Ehdr *
construct_ehdr(void)
{
    return malloc(sizeof(struct My_Ehdr));
}

// destructor
void destruct_ehdr(My_Ehdr *hdr)
{

    if (!hdr)
    {
        write(2, "you might have passed NULL elf structure\n", strlen("you might have passed NULL elf structure\n"));
        exit(EXIT_FAILURE);
    }

    free(hdr);
    hdr = NULL;
}

// caller of all checks
bool elf_check_all(const char *lib, My_Ehdr *hdr)
{

    if (!hdr)
    {
        write(2, "you might have passed NULL elf structure\n", strlen("you might have passed NULL elf structure\n"));
        return false;
    }

    //
    FILE *elf_file = fopen(lib, "rb");
    if (!elf_file)
    {
        write(2, "Failed to open file\n", strlen("Failed to open file\n"));

        return false;
    }

    if (fread(hdr, sizeof(*hdr), 1, elf_file) != 1)
    {
        write(2, "Failed to read ELF header\n", strlen("Failed to read ELF header\n"));
        fclose(elf_file);
        elf_file = NULL;
        return false;
    }

    fclose(elf_file);
    elf_file = NULL;
    //

    if (!elf_check_magic(hdr))
        return false;

    if (!elf_check_arch(hdr))
        return false;

    if (!elf_check_type(hdr))
        return false;

    if (!elf_check_size(hdr))
        return false;

    if (!elf_check_segments(hdr))
        return false;

    return true;
}

//***
// GETTERS

uint16_t
get_ephsize(const My_Ehdr *hdr)
{

    if (!hdr)
    {
        write(2, "you might have passed NULL elf structure\n", strlen("you might have passed NULL elf structure\n"));
        exit(EXIT_FAILURE);
    }

    return hdr->e_phentsize;
}

uint16_t
get_ephnum(const My_Ehdr *hdr)
{

    if (!hdr)
    {
        write(2, "you might have passed NULL elf structure\n", strlen("you might have passed NULL elf structure\n"));
        exit(EXIT_FAILURE);
    }

    return hdr->e_phnum;
}

uint64_t
get_phoffest(const My_Ehdr *hdr)
{

    if (!hdr)
    {
        write(2, "you might have passed NULL elf structure\n", strlen("you might have passed NULL elf structure\n"));
        exit(EXIT_FAILURE);
    }

    return hdr->e_phoff;
}

uint64_t get_eentry(const My_Ehdr *hdr)
{

    if (!hdr)
    {
        write(2, "you might have passed NULL elf structure\n", strlen("you might have passed NULL elf structure\n"));
        exit(EXIT_FAILURE);
    }

    return hdr->e_entry;
}
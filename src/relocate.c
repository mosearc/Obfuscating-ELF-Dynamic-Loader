#include "../include/relocate.h"

/* IN THIS FILE:
 *====================================
 * CHALLENGE 5
 *====================================
 */

// One entry of the .dynamic table
// d_tag: entry kind (DT_RELA, DT_RELASZ, ...)
// d_val: value (address or size depending on the tag)
typedef struct
{
    int64_t d_tag;
    uint64_t d_val;
} My_Dyn;

// One Rela relocation entry.
// r_offset: where to patch (vaddr, will be added to base_address)
// r_info: encodes both the symbol index and the relocation type
// r_addend: value to add to base_address for the relocation
typedef struct
{
    uint64_t r_offset;
    uint64_t r_info;
    int64_t r_addend;
} My_Rela;

// Determine whether the segment containing 'vaddr' is read-only.
__attribute__((nonnull(1, 2))) static int is_readonly(const My_Ehdr *ehdr, const char *lib, uint64_t vaddr)
{
    My_Phdr *tmp = construct_phdr();
    if (!tmp)
        return -1;

    FILE *f = fopen(lib, "rb");
    if (!f)
    {
        destruct_phdr(tmp);
        return -1;
    }

    if (fseek(f, get_phoffest(ehdr), SEEK_SET) != 0)
    {
        fclose(f);
        destruct_phdr(tmp);
        return -1;
    }

    int result = -1;

    for (int i = 0; i < get_ephnum(ehdr); i++)
    {
        if (fread(tmp, get_ephsize(ehdr), 1, f) != 1)
        {
            fclose(f);
            destruct_phdr(tmp);
            return -1;
        }

        if (get_ptype(tmp) != PT_LOAD)
            continue;

        uint64_t seg_start = get_pvaddr(tmp);
        uint64_t seg_end = seg_start + get_pmemsz(tmp);

        if (vaddr >= seg_start && vaddr < seg_end)
        {
            if (get_pflag(tmp) & PF_W)
                result = 0; // writable
            else
                result = 1; // read-only
            break;
        }
    }

    fclose(f);
    destruct_phdr(tmp);
    return result;
}

// Apply the relocations of the loaded library.
int relocate(const My_Ehdr *ehdr, const My_Phdr *phdr, const char *lib, const uint64_t base_address, const long page_size)
{
    if (!phdr || !ehdr)
    {
        write(2, "you might have passed NULL structure\n", strlen("you might have passed NULL structure\n"));
        return 0;
    }

    // The .dynamic table is reachable through (base + p_vaddr) since the memory should be already allocated we can cast it
    My_Dyn *dyn = (My_Dyn *)(base_address + get_pvaddr(phdr));

    uint64_t rela_addr = 0;  // virtual address of the .rela.dyn array
    uint64_t rela_count = 0; // number of entries (DT_RELACOUNT)

    bool found_rela = false;
    bool found_relacount = false;

    for (int i = 0; dyn[i].d_tag != DT_NULL; i++)
    {
        switch (dyn[i].d_tag)
        {
        case DT_RELA:
            rela_addr = dyn[i].d_val;
            found_rela = true;
            break;
        case DT_RELACOUNT:
            rela_count = dyn[i].d_val;
            found_relacount = true;
            break;
        }
    }

    // Nothing to relocate
    if (!found_rela && !found_relacount)
    {
        return 1;
    }

    if (rela_count <= 0)
    {
        return 0;
    }

    uint64_t num_entries = rela_count;
    My_Rela *rela = (My_Rela *)(base_address + rela_addr);

    for (uint64_t i = 0; i < num_entries; i++)
    {
        uint32_t type = ELF64_R_TYPE(rela[i].r_info);

        // check architecture
        if (type != R_X86_64_RELATIVE && type != R_AARCH64_RELATIVE)
        {
            continue;
        }

        // Real address inside the process: where we will write.
        uint64_t real_addr = base_address + rela[i].r_offset;

        int readonly = is_readonly(ehdr, lib, rela[i].r_offset);

        if (readonly == -1)
        {
            write(2, "Could not find segment for relocation\n", strlen("Could not find segment for relocation\n"));
            return 0;
        }

        if (readonly)
        {
            uint64_t page_start = (real_addr / page_size) * page_size;
            if (mprotect((void *)page_start, page_size, PROT_READ | PROT_WRITE) == -1)
            {
                write(2, "mprotect write fail\n", strlen("mprotect write fail\n"));
                return 0;
            }
        }

        // Actual relocation patch
        uint64_t *relative = (uint64_t *)(real_addr);
        *relative = base_address + rela[i].r_addend;

        // Restore the original protections.
        if (readonly)
        {

            int prot = 0;
            if (get_pflag(phdr) & PF_R)
                prot |= PROT_READ;
            if (get_pflag(phdr) & PF_W)
                prot |= PROT_WRITE;
            if (get_pflag(phdr) & PF_X)
                prot |= PROT_EXEC;

            uint64_t page_start = (real_addr / page_size) * page_size;
            if (mprotect((void *)page_start, page_size, prot) == -1)
            {
                write(2, "mprotect restore fail\n", strlen("mprotect restore fail\n"));
                return 0;
            }
        }
    }

    return 1;
}

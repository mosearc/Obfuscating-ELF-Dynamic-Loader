#include "../include/mmapp.h"

/* IN THIS FILE:
 *====================================
 * CHALLENGE 4
 *====================================
 */

// Reserve the whole virtual range that will hold every PT_LOAD segment.
uint64_t memory_reserving(const uint64_t total_mem)
{

    if (total_mem <= 0)
    {
        write(2, "Initial mmap failed\n", strlen("Initial mmap failed\n"));
        return UINT64_MAX;
    }

    // 0x00 as a hint => let the kernel choose. PROT_NONE => fail-safe.
    void *mapping = mmap(0x00, total_mem, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

    if (mapping == MAP_FAILED)
    {
        write(2, "Initial mmap failed\n", strlen("Initial mmap failed\n"));
        return UINT64_MAX;
    }

    // The mapping pointer corresponds to the first PT_LOAD's vaddr. Subtracting that vaddr therefore results in the constant offset the linker effectively needs to add to every recorded address.
    uint64_t base_address = (uint64_t)mapping - get_pfirstv();
    return base_address;
}

// Map a single PT_LOAD segment from the ELF file into the reservation.
int segment_loading(const My_Phdr *phdr, const int fd, const uint64_t base_address, const long page_size)
{
    if (!phdr || fd < 0 || page_size < 0 || base_address == UINT64_MAX)
    {
        write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
        return 0;
    }

    // Real (unaligned) address where the segment's content should land.
    uint64_t real_seg_addr = base_address + get_pvaddr(phdr);
    // Offset from the page start to the actual segment content.
    uint64_t segment_page_offst = get_pvaddr(phdr) % page_size;

    // Page-aligned target address and file offset (both rounded down).
    uint64_t aligned_real_addr = real_seg_addr - segment_page_offst;
    uint64_t aligned_real_offst = get_ppoffset(phdr) - (get_ppoffset(phdr) % page_size);

    // Length must include the rounding delta we added at the start.
    uint64_t segment_file_size = get_pfilesz(phdr) + segment_page_offst;

    // Skip the file-backed mmap for pure-BSS segments (p_filesz == 0).
    if (get_pfilesz(phdr) > 0)
    {
        // MAP_FIXED: we must land exactly inside the reservation. PROT_NONE: protection is set later by segment_protceting().
        void *mapped = mmap((void *)aligned_real_addr, segment_file_size, PROT_NONE, MAP_PRIVATE | MAP_FIXED, fd, aligned_real_offst);
        if (mapped == MAP_FAILED)
        {
            write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
            return 0;
        }
    }

    // --- BSS: zero-fill anything beyond p_filesz up to p_memsz ---
    if (get_pmemsz(phdr) > get_pfilesz(phdr))
    {
        uint64_t file_end = base_address + get_pvaddr(phdr) + get_pfilesz(phdr);   // end of file content, after this bss
        uint64_t next_page = ((file_end + page_size - 1) / page_size) * page_size; // next page boundary up
        uint64_t seg_end = base_address + get_pvaddr(phdr) + get_pmemsz(phdr);     // end fo the whole memsz

        // Make the tail-of-file page writable so we can memset() the remaining  bytes to zero
        int mapprotect = mprotect((void *)((file_end / page_size) * page_size), page_size, PROT_WRITE);
        if (mapprotect == -1)
        {
            write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
            return 0;
        }

        memset((void *)file_end, 0, next_page - file_end);

        // If BSS spans additional pages, back them with anonymous memory.
        if (seg_end > next_page)
        {
            void *mapped2 = mmap((void *)next_page, seg_end - next_page, PROT_NONE, MAP_PRIVATE | MAP_FIXED | MAP_ANON, -1, 0);
            if (mapped2 == MAP_FAILED)
            {
                write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
                return 0;
            }
        }
    }

    return 1;
}

// Apply the final R/W/X protection of a PT_LOAD segment.
int segment_protceting(const My_Phdr *phdr, const uint64_t base_address, const long page_size)
{
    if (!phdr || page_size < 0 || base_address == UINT64_MAX)
    {
        write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
        return 0;
    }

    uint64_t real_seg_addr = base_address + get_pvaddr(phdr);
    uint64_t segment_page_offst = get_pvaddr(phdr) % page_size;

    uint64_t aligned_real_addr = real_seg_addr - segment_page_offst;

    // Calculate and round the total in-memory size up to a whole number of pages.
    uint64_t segment_real_size = ((get_pmemsz(phdr) + segment_page_offst + page_size - 1) / page_size) * page_size;

    // Translate p_flags (PF_*) into mmap/mprotect flags (PROT_*).
    int prot = 0;
    if (get_pflag(phdr) & PF_R)
        prot |= PROT_READ;
    if (get_pflag(phdr) & PF_W)
        prot |= PROT_WRITE;
    if (get_pflag(phdr) & PF_X)
        prot |= PROT_EXEC;

    int mapprotect = mprotect((void *)aligned_real_addr, segment_real_size, prot);
    if (mapprotect == -1)
    {
        write(2, "Segment mmap failed\n", strlen("Segment mmap failed\n"));
        return 0;
    }

    return 1;
}

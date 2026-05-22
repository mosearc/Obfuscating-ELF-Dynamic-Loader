#include "../include/my_dl.h"
//#include <dlfcn.h>

/* IN THIS FILE:
 *====================================
 * MY_DLOPEN AND MY_DLSYM
 *====================================
 */

void *my_dlopen(const char *dllib)
{

    //--- ch2: ELF header parsing & sanity checks---
    My_Ehdr *hdr = construct_ehdr();
    if (!hdr)
    {
        write(2, "Error in allocating my_ehdr\n", strlen("Error in allocating my_ehdr\n"));
        return NULL;
    }

    if (!elf_check_all(dllib, hdr))
    {

        write(2, "Error ELF Header\n", strlen("Error ELF Header\n"));
        destruct_ehdr(hdr);
        hdr = NULL;
        return NULL;
    }

    //--- ch3: program headers parsing & PT_LOAD layout checks ---
    My_Phdr *pdr = construct_phdr();
    if (!pdr)
    {
        write(2, "Error in allocating my_phdr\n", strlen("Error in allocating my_phdr\n"));
        destruct_ehdr(hdr);
        return NULL;
    }

    if (!p_check_all(pdr, hdr, dllib))
    {
        write(2, "Error Program Header\n", strlen("Error Program Header\n"));
        destruct_phdr(pdr);
        destruct_ehdr(hdr);
        pdr = NULL;
        return NULL;
    }

    //--- ch4 + ch5: map segments + perform relocations ---
    void *handle = load_in_mem(hdr, pdr, dllib);

    if (!handle)
    {
        write(2, "Error in Handle creation\n", strlen("Error in Handel creation\n"));
        return NULL;
    }

    //--- ch1 (kept for reference): libdl-based implementation. ---
    // void *handle = dlopen(dllib, RTLD_LAZY);

    // if (!handle)
    // {
    //     fprintf(stderr, "Error dlopen: %s\n", dlerror());
    //     return NULL;
    // }

    // return handle;

    //--- ch6: build the opaque handle exposed to the caller ---
    My_handle *dl = malloc(sizeof(My_handle));
    if (!dl)
    {
        write(2, "error\n", strlen("error\n"));
        return NULL;
    }

    dl->base = handle;
    dl->symtab = (My_sentry *)((uint64_t)handle + get_eentry(hdr));

    destruct_ehdr(hdr);
    destruct_phdr(pdr);
    hdr = NULL;
    pdr = NULL;

    return dl;
}

void *my_dlsym(void *handle, const char *str)
{

    if (!handle)
    {

        write(2, "errordlsym\n", strlen("errordlsym\n"));
        return NULL;
    }

    My_handle *dl = (My_handle *)handle;

    for (My_sentry *s = dl->symtab; s->name != NULL; s++)
    {

        if (strcmp(s->name, str) == 0)
        {
            return s->addr;
        }
    }

    write(2, "symbol not found\n", strlen("symbol not found\n"));

    return NULL;
}

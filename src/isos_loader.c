#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include "../include/my_dl.h"
#include "../include/my_argp.h"

/* IN THIS FILE:
 *====================================
 * THE ISOS_LOADER MAIN PROGRAM
 *====================================
 */

#define KEY 0x42

//-- Bonus 2 --
// decript and extract the hidden library
static char *extract_lib(const char *self)
{
    FILE *f = fopen(self, "rb");
    if (!f)
    {
        write(2, "fopen self failed\n", strlen("fopen self failed\n"));
        return NULL;
    }

    // Read the size footer (last 8 bytes of the file).
    if (fseek(f, -8, SEEK_END) != 0)
    {
        write(2, "fseek footer failed\n", strlen("fseek footer failed\n"));
        fclose(f);
        return NULL;
    }

    long size;
    if (fread(&size, 8, 1, f) != 1)
    {
        write(2, "fread size failed\n", strlen("fread size failed\n"));
        fclose(f);
        return NULL;
    }

    // Position right at the start of the encrypted blob: size+8 from EOF.
    if (fseek(f, -(8 + size), SEEK_END) != 0)
    {
        write(2, "fseek blob failed\n", strlen("fseek blob failed\n"));
        fclose(f);
        return NULL;
    }

    /// tmp endzone for the decrypted .so
    char *lib = "/tmp/mylib_decrypted";

    FILE *out = fopen(lib, "wb");
    if (!out)
    {
        write(2, "fdopen failed\n", strlen("fdopen failed\n"));
        fclose(f);
        return NULL;
    }

    for (long i = 0; i < size; i++)
    {
        int c = fgetc(f);
        if (c == EOF)
        {
            write(2, "fgetc failed\n", strlen("fgetc failed\n"));
            fclose(out);
            fclose(f);
            return NULL;
        }
        if (fputc(c ^ KEY, out) == EOF)
        {
            write(2, "fputc failed\n", strlen("fputc failed\n"));
            fclose(out);
            fclose(f);
            return NULL;
        }
    }

    fclose(out);
    fclose(f);
    return lib;
}

int main(int argc, char *argv[])
{
    struct arguments args = {0};

    args.functions = malloc((argc - 1) * sizeof(char *));

    parsing(argc, argv, &args);

    // Recover the embedded shared library from our own binary.
    char *path = extract_lib(argv[0]);

    if (!path)
    {
        free(args.functions);
        return EXIT_FAILURE;
    }

    // custom handle opaque pointer
    My_handle *handle = my_dlopen(path);

    if (!handle)
    {
        free(args.functions);
        return EXIT_FAILURE;
    }

    // Loop the requested function names, resolve each one through the custom symbol table and invoke it.
    for (int i = 0; i < args.fcount; i++)
    {
        char *(*func)(void) = (char *(*)(void))my_dlsym(handle, args.functions[i]);
        if (!func)
        {

            write(2, "\t\t\tskipped\n\n", strlen("\t\t\tskipped\n\n"));

            continue;
        }
        func();
        printf("\n%s -- ok\n\n", args.functions[i]);
    }

    free(args.functions);
    munmap((void *)((uint64_t)handle->base + get_pfirstv()), get_mmap_totmem());
    free(handle);
    return EXIT_SUCCESS;
}

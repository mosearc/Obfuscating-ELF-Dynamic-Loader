#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* IN THIS FILE:
 *====================================
 * BONUS 2
 *====================================
 */

#define KEY 0x42

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        write(2, "usage: encrypter <loader_binary> <plain_lib.so>\n", strlen("usage: encrypter <loader_binary> <plain_lib.so>\n"));
        exit(EXIT_FAILURE);
    }

    FILE *dst = fopen(argv[1], "ab"); // isos_loader
    FILE *src = fopen(argv[2], "rb"); // library

    if (!src || !dst)
    {
        write(2, "problems in managing the lib\n", strlen("problems in managing the lib\n"));
        exit(EXIT_FAILURE);
    }

    // Stream the source byte by byte, XOR-ing each one with the key. Saving the size
    long size = 0;
    int c;
    while ((c = fgetc(src)) != EOF)
    {
        fputc(c ^ KEY, dst);
        size++;
    }

    // Append the size as a footer so the loader knows how far to seek back.
    fwrite(&size, 8, 1, dst);

    fclose(src);
    fclose(dst);
    return 0;
}
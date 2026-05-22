#ifndef MY_ARGP_H
#define MY_ARGP_H

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct arguments
{
    char **functions;
    int fcount;
};

void parsing(int argc, char **argv, struct arguments *args);

#endif
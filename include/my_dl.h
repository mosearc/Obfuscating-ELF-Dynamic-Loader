#ifndef MY_DL_H
#define MY_DL_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../include/my_elfh.h"
#include "../include/my_ph.h"

void *my_dlopen(const char *dllib);

void *my_dlsym(void *handle, const char *str);

// One entry of the custom symbol table that the shared library exposes.
typedef struct
{
    const char *name;
    void *addr;
} My_sentry;

// Opaque handle returned by my_dlopen().
typedef struct
{
    void *base;
    My_sentry *symtab;
} My_handle;

#endif
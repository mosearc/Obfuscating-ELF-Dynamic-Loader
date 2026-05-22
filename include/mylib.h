#ifndef MYLIB_H
#define MYLIB_H

#include "../include/my_dl.h"

char *foo_exported(void);
char *bar_exported(void);

char *inside_foo(void);
char *inside_bar(void);

#endif
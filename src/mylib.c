#include "../include/mylib.h"

/* IN THIS FILE:
 *====================================
 * SHRED LIBRARY THAT THE ISOS_LOADER WILL LOAD
 *====================================
 */

char *foo_exported(void)
{
    char *literalFoo = "foo";
    return literalFoo;
}

char *bar_exported(void)
{
    char *literalBar = "bar";
    return literalBar;
}

// --- Reference to a previous design attempt (kept for context). ---
//
// __attribute__((weak)) My_sentry _start[] = { //use the compilaton flag instead of weak
//     {"foo_exported", (void *)foo_exported},
//     {"bar_exported", (void *)bar_exported},
//     {NULL, NULL}};

My_sentry tableS[] = {
    {"foo_exported", (void *)foo_exported},
    {"bar_exported", (void *)bar_exported},
    {NULL, NULL}};

// char* foo_imported(void){
//     return inside_foo();
// }

// char* bar_imported(void){
//     return inside_bar();
// }

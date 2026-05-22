#include "../include/mylib.h"

/* IN THIS FILE:
 *====================================
 * CHALLENGE 0
 *====================================
 */

char *inside_foo(void)
{
    char *a = "42";
    return a;
}

char *inside_bar(void)
{
    char *a = "47";
    return a;
}

int main()
{
    char *fool = foo_exported();
    char *barl = bar_exported();

    return 0;
}
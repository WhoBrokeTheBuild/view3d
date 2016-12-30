#include "util.h"

#include <debug.h>

long int fsize(FILE *fp)
{
    long int start, size;

    CHECK(fp, "File not open");

    start = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, start, SEEK_SET);

    return size;

error:

    return -1;
}

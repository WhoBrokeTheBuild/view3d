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

void calc_normal(vec3f_t normal, vec3f_t a, vec3f_t b, vec3f_t c)
{
    vec3f_t tmpa, tmpb;

    vec3f_sub(tmpa, b, a);
    vec3f_sub(tmpb, c, a);
    vec3f_cross(normal, tmpa, tmpb);
}

char *_strdup(const char *str)
{
    size_t len;
    char *copy = NULL;

    if (!str)
    {
        return NULL;
    }

    len = strlen(str) + 1;
    copy = malloc(len);
    if (!copy)
    {
        return NULL;
    }

    memcpy(copy, str, len);
    return (copy);
}

char *_strndup(const char *str, size_t n)
{
    char *result = NULL;
    size_t len;

    if (!str)
    {
        return NULL;
    }

    len = strlen(str);

    if (n < len)
    {
        len = n;
    }

    result = (char *)malloc(len + 1);
    if (!result)
    {
        return NULL;
    }

    result[len] = '\0';
    return (char *)memcpy(result, str, len);
}

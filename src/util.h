#ifndef VIEW3D_UTIL_H
#define VIEW3D_UTIL_H

#include <string.h>
#include <view3d.h>

long int fsize(FILE *fp);

void calc_normal(vec3f_t normal, vec3f_t a, vec3f_t b, vec3f_t c);

unsigned int strcntchr(const char *str, char c);
char *_strdup(const char *str);
char *_strndup(const char *str, size_t n);

#endif // VIEW3D_UTIL_H

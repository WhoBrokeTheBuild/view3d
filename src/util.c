#include "util.h"

#include "debug.h"

long int fsize(FILE *fp)
{
  long int start, size;

  assert(fp);

  start = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fseek(fp, start, SEEK_SET);

  return size;
}

void calc_normal(vec3f_t normal, vec3f_t a, vec3f_t b, vec3f_t c)
{
  vec3f_t tmpa, tmpb;

  vec3f_xsub(tmpa, b, a);
  vec3f_xsub(tmpb, c, a);
  vec3f_xcross(normal, tmpa, tmpb);
}

unsigned int strcntchr(const char *str, char c)
{
  int i;
  unsigned int count = 0;

  for (i = 0; str[i]; ++i)
  {
    if (str[i] == c)
    {
      ++count;
    }
  }

  return count;
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

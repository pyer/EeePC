/* Public domain. */

#include "str.h"

size_t str_len(const char *s)
{
  if (!s)
    return(0);
  register const char *t = s;
  while(*t)
    t++;
  return(t-s);
}

/*
unsigned int str_len(const char *s)
{
  register const char *t;

  t = s;
  for (;;) {
    if (!*t) return t - s; ++t;
    if (!*t) return t - s; ++t;
    if (!*t) return t - s; ++t;
    if (!*t) return t - s; ++t;
  }
}
*/


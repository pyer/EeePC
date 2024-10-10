/* Public domain. */

#include "str.h"

int str_equal(register const char *s,register const char *t)
{
  if (s==0 || t==0)
      return(0); // false

  while (*s && *t) {
    if (*s != *t) {
      return(0); // false
    }
    s++;
    t++;
  }
  if (*s)
      return(0); // false
  if (*t)
      return(0); // false

  return(1); // true
}


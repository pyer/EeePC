#include <assert.h>
#include <stdio.h>
//#include <unistd.h>

#include "../src/str.h"

/*
 * main entry
 */
int main(int argc, char *argv[]) {
  char s[10] = "control/?";
  s[8] = 'A';
  assert( str_len(s) == 9 );
  assert( str_equal(s, "control/A") );


  assert( str_len(0) == 0);
  assert( str_len("") == 0);
  assert( str_len("a") == 1);

  assert( str_len("abcdefg") == 7);

  assert( str_equal("", "") );
  assert( str_equal("abc", "abc") );

  assert( !str_equal(0, "abc") );
  assert( !str_equal("abc", 0) );

  assert( !str_equal("abc", "abcdef") );
  assert( !str_equal("abc", "") );
  assert( !str_equal("", "abcdef") );
  assert( !str_equal("abcdef", "abc") );
  assert( !str_equal("abc", "abcdef") );

  return(0);
}


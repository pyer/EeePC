#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../src/log.h"

//char *result_name = "./result";
  /*
  int fd = creat(result_name, 0644);
  if (fd < 0 ) {
    puts( "ERROR: unable to create file");
  }
  close(fd);
*/

/*
 * main entry
 */
int main(int argc, char *argv[]) {
  size_t n = wlen("abcd");
  assert(n == 4);

  log_info(0, 0, 0);
  log_info("a", 0, 0);
  log_info("a", "b", 0);
  log_info("a", "b", "c");

  log_warn(0, 0, 0);
  log_warn("a", 0, 0);
  log_warn("a", "b", 0);
  log_warn("a", "b", "c");

  errno = 1;
  log_error("a", "b", " ");
  errno = 0;

  return(0);
}


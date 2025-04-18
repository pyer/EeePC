/* Public domain. */

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int wait_pid(wstat,pid) int *wstat; int pid;
{
  int r;

  do
    r = waitpid(pid,wstat,0);
  while ((r == -1) && (errno == EINTR));
  return r;
}

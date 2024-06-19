#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "byte.h"
#include "env.h"
#include "error.h"
#include "str.h"
#include "strerr.h"

#include "alloc.h"
#include "stralloc.h"

#define FATAL "[ SETSID ] fatal: "

const char *progname;
static stralloc tmp;

int main(int argc, char * const *argv) {
  const char *path;
  unsigned int split;
  int savederrno;

  setsid();

  argv++;
  progname = *argv;

/*
  pathexec_run(progname,argv,environ);
void pathexec_run(const char *file,const char * const *argv,const char * const *envp)
*/

  if (progname[str_chr(progname,'/')]) {
    execve(progname,argv,environ);
    return(0);
  }

  path = env_get("PATH");
  if (!path) path = "/bin:/usr/bin";

  savederrno = 0;
  for (;;) {
    split = str_chr(path,':');
    if (!stralloc_copyb(&tmp,path,split)) break;
    if (!split)
      if (!stralloc_cats(&tmp,".")) break;
    if (!stralloc_cats(&tmp,"/"))  break;
    if (!stralloc_cats(&tmp,progname)) break;
    if (!stralloc_0(&tmp)) break;

    execve(tmp.s,argv,environ);
    if (errno != error_noent) {
      savederrno = errno;
      if ((errno != error_acces) && (errno != error_perm) && (errno != error_isdir)) break;
    }

    if (!path[split]) {
      if (savederrno) errno = savederrno;
      break;
    }
    path += split;
    path += 1;
  }

  strerr_die5sys(111, FATAL, "unable to run", ": ", *argv, ": ");
  return(0);
}


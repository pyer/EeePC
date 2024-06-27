/* runsvdir.c */
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "strerr.h"
#include "error.h"
#include "wait.h"
#include "env.h"
#include "open.h"
#include "fd.h"
#include "str.h"
#include "iopause.h"
#include "sig.h"
#include "ndelay.h"

#define MAXSERVICES 1000

char *svdir = "/etc/svdir";

unsigned long dev =0;
unsigned long ino =0;
struct {
  unsigned long dev;
  unsigned long ino;
  int pid;
  int isgone;
} sv[MAXSERVICES];
int svnum =0;
int check =1;
iopause_fd io[1];
int exitsoon =0;

void fatal(char *m1, char *m2) {
  strerr_die6sys(100, "runsvdir ", svdir, ": fatal: ", m1, m2, ": ");
}
void warn(char *m1, char *m2) {
  strerr_warn6("runsvdir ", svdir, ": warning: ", m1, m2, ": ", &strerr_sys);
}
void warn3x(char *m1, char *m2, char *m3) {
  strerr_warn6("runsvdir ", svdir, ": warning: ", m1, m2, m3, 0);
} 
void s_term()   { exitsoon = 1; }
void s_hangup() { exitsoon = 2; }

int a_dot_is_in( char *p) {
  while (*p) {
    if (*p == '.') {
      return 1;
    }
    p++;
  }
  return 0;
}

void runsv(int no, char *name) {
  int pid;

  if ((pid = fork()) == -1) {
    warn("unable to fork for ", name);
    return;
  }
  if (pid == 0) {
    /* child */
    const char *prog[3];

    prog[0] = "/sbin/runsv";
    prog[1] = name;
    prog[2] = 0;
    sig_uncatch(SIGHUP);
    sig_uncatch(SIGTERM);
    setsid();
    execve(*prog, (char * const*)prog, (char* const*)environ);
    fatal("unable to start runsv ", name);
  }
  sv[no].pid = pid;
}

void runsvdir() {
  DIR *dir;
  struct dirent *d;
  int i;
  struct stat s;

  if (! (dir =opendir("."))) {
    warn("unable to open directory ", svdir);
    return;
  }
  for (i =0; i < svnum; i++)
    sv[i].isgone = 1;

  errno =0;
  while ((d = readdir(dir))) {
    if ( a_dot_is_in(d->d_name) )
      continue;
    if (stat(d->d_name, &s) == -1) {
      warn("unable to stat ", d->d_name);
      errno =0;
      continue;
    }
    if (! S_ISDIR(s.st_mode))
      continue;
    for (i =0; i < svnum; i++) {
      if ((sv[i].ino == s.st_ino) && (sv[i].dev == s.st_dev)) {
        sv[i].isgone =0;
        if (! sv[i].pid) runsv(i, d->d_name);
        break;
      }
    }
    if (i == svnum) {
      /* new service */
      if (svnum >= MAXSERVICES) {
        warn3x("unable to start runsv ", d->d_name, ": too many services.");
        continue;
      }
      sv[i].ino =s.st_ino;
      sv[i].dev =s.st_dev;
      sv[i].pid =0;
      sv[i].isgone =0;
      svnum++;
      runsv(i, d->d_name);
      check =1;
    }
  }
  if (errno) {
    warn("unable to read directory ", svdir);
    closedir(dir);
    check =1;
    return;
  }
  closedir(dir);

  /* SIGTERM removed runsv's */
  for (i = 0; i < svnum; i++) {
    if (! sv[i].isgone)
      continue;
    if (sv[i].pid)
      kill(sv[i].pid, SIGTERM);

    sv[i] = sv[--svnum];
    check =1;
  }
}

int main(int argc, char **argv) {
  struct stat s;
  time_t mtime =0;
  int wstat;
  int curdir;
  int pid;
  struct taia deadline;
  struct taia now;
  struct taia stampcheck;
  int i;

  if (chdir(svdir) != 0) {
    fatal("unable to change directory to ", svdir);
  }

  sig_catch(SIGTERM, s_term);
  sig_catch(SIGHUP, s_hangup);
  if ((curdir =open_read(".")) == -1) 
    fatal("unable to open current directory", 0);
  fcntl(curdir,F_SETFD,1);

  taia_now(&stampcheck);

  for (;;) {
    /* collect children */
    for (;;) {
      if ((pid =wait_nohang(&wstat)) <= 0) break;
      for (i =0; i < svnum; i++) {
        if (pid == sv[i].pid) {
          /* runsv has gone */
          sv[i].pid =0;
          check =1;
          break;
        }
      }
    }

    taia_now(&now);
    if (now.sec.x < (stampcheck.sec.x -3)) {
      /* time warp */
      warn3x("time warp: resetting time stamp.", 0, 0);
      taia_now(&stampcheck);
      taia_now(&now);
    }
    if (taia_less(&now, &stampcheck) == 0) {
      /* wait at least a second */
      taia_uint(&deadline, 1);
      taia_add(&stampcheck, &now, &deadline);
      
      if (stat(svdir, &s) != -1) {
        if (check || s.st_mtime != mtime || s.st_ino != ino || s.st_dev != dev) {
            mtime =s.st_mtime;
            dev =s.st_dev;
            ino =s.st_ino;
            check =0;
            if (now.sec.x <= (4611686018427387914ULL +(uint64)mtime))
              sleep(1);
            runsvdir();
        }
      } else {
        warn("unable to stat ", svdir);
      }
    }

    taia_uint(&deadline, check ? 1 : 5);
    taia_add(&deadline, &now, &deadline);

    sig_block(SIGCHLD);
    iopause(0, 0, &deadline, &now);
    sig_unblock(SIGCHLD);

    switch(exitsoon) {
    case 1:
      _exit(0);
    case 2:
      for (i =0; i < svnum; i++) if (sv[i].pid) kill(sv[i].pid, SIGTERM);
      _exit(111);
    }
  }
  /* not reached */
  _exit(0);
}

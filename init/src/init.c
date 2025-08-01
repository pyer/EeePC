/* init.c */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "iopause.h"
#include "log.h"
#include "sig.h"
#include "str.h"
#include "wait.h"

#define STARTUP  "/etc/startup"
#define SHUTDOWN "/etc/shutdown"
#define SVDIR    "/etc/svdir/enabled"

#define HALT     "/run/shutdown.halt"
#define POWEROFF "/run/shutdown.poweroff"
#define REBOOT   "/run/shutdown.reboot"

#define LOG_PREFIX "init"

#define INFO "[ INIT ] "
#define WARNING "[ INIT ] warning: "
#define FATAL "[ INIT ] fatal: "

#define MAXSERVICES 1000
char *svdir = SVDIR;

int reboot_mode = 0;

int sigc =0;
int sigi =0;

/*
 * signal handlers
 */
void sig_cont_handler(void) {
  log_info(LOG_PREFIX, "SIGCONT\n", 0);
  sigc++;
}

void sig_int_handler(void) {
  log_info(LOG_PREFIX, "SIGINT\n", 0);
  sigi++;
}

/*
void sig_child_handler(void) {
  log_info(LOG_PREFIX, "SIGCHILD", 0);
}
*/

/*
 * run services
 */
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

void runsv(int no, char *name, char * const *envp)
{
  int pid;

  if ((pid = fork()) == -1) {
    log_warn(LOG_PREFIX, "unable to fork for ", name);
    return;
  }
  if (pid == 0) {
    /* child */
    const char *prog[3];
    prog[0] = "/sbin/runsv";
    prog[1] = name;
    prog[2] = 0;
    sig_default(SIGHUP);
    sig_default(SIGTERM);
    setsid();
    execve(*prog, (char * const*)prog, envp);
    log_error(LOG_PREFIX, "unable to start runsv ", name);
    _exit(100);
  }
  sv[no].pid = pid;
}

void runsvdir(char * const *envp)
{
  DIR *dir;
  struct dirent *d;
  int i;
  struct stat s;

  if (! (dir = opendir("."))) {
    log_warn(LOG_PREFIX, "unable to open directory ", svdir);
    return;
  }
  for (i =0; i < svnum; i++)
    sv[i].isgone = 1;

  errno = 0;
  while ((d = readdir(dir))) {
    if ( d->d_name[0] == '.' )
      continue;
    if (stat(d->d_name, &s) == -1) {
      log_warn(LOG_PREFIX, "unable to stat ", d->d_name);
      errno = 0;
      continue;
    }
    if (! S_ISDIR(s.st_mode))
      continue;
    for (i =0; i < svnum; i++) {
      if ((sv[i].ino == s.st_ino) && (sv[i].dev == s.st_dev)) {
        sv[i].isgone =0;
        if (! sv[i].pid) runsv(i, d->d_name, envp);
        break;
      }
    }
    if (i == svnum) {
      /* new service */
      if (svnum >= MAXSERVICES) {
        log_warn(LOG_PREFIX, "too many services: unable to start ", d->d_name);
        continue;
      }
      sv[i].ino =s.st_ino;
      sv[i].dev =s.st_dev;
      sv[i].pid =0;
      sv[i].isgone =0;
      svnum++;
      runsv(i, d->d_name, envp);
      check =1;
    }
  }
  if (errno) {
    log_warn(LOG_PREFIX, "unable to read directory ", svdir);
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

/*
 */
int services(char * const *envp) {
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
    log_error(LOG_PREFIX, "unable to change directory to ", svdir);
    return(100);
  }

  if ((curdir = open("." ,O_RDONLY | O_NDELAY)) == -1) {
    log_error(LOG_PREFIX, "unable to open current directory", 0);
    return(100);
  }
  fcntl(curdir,F_SETFD,1);

  setsid();

      sig_unblock(SIGALRM);
      sig_unblock(SIGCHLD);
      sig_default(SIGCHLD);
      sig_unblock(SIGCONT);
      sig_catch(SIGCONT, sig_cont_handler);
      sig_unblock(SIGHUP);
      sig_unblock(SIGINT);
      sig_catch(SIGINT, sig_int_handler);
      sig_unblock(SIGPIPE);
      sig_unblock(SIGTERM);

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
      log_warn(LOG_PREFIX, "time warp", ": resetting time stamp.");
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
            runsvdir(envp);
        }
      } else {
        log_warn(LOG_PREFIX, "unable to stat ", svdir);
      }
    }

    taia_uint(&deadline, check ? 1 : 5);
    taia_add(&deadline, &now, &deadline);

    sig_block(SIGCHLD);
    iopause(0, 0, &deadline, &now);
    sig_unblock(SIGCHLD);

    if (sigi) {
        log_info(LOG_PREFIX, "ctrl-alt-del request...", 0);
        sigi = 0;
        sigc++;
    }
    if (sigc) {
        log_info(LOG_PREFIX, "shutdown request...", 0);
        /* choose reboot mode */
        reboot_mode = RB_AUTOBOOT;
        struct stat rstat;
        if (stat(POWEROFF, &rstat) == 0)
          reboot_mode = RB_POWER_OFF;
        if (stat(HALT, &rstat) == 0)
          reboot_mode = RB_HALT_SYSTEM;

        sigc =0;
//        for (i =0; i < svnum; i++) if (sv[i].pid) kill(sv[i].pid, SIGTERM);
        return(111);
    }
  }
  /* not reached */
  return(0);
}

void execute(char *action, char * const *envp)
{
    int pid;
    int wstat;
    int ttyfd;
    const char * prog[2];
    log_info(LOG_PREFIX, "enter ", action);

    while ((pid = fork()) == -1) {
      log_warn(LOG_PREFIX, "unable to fork for ", SHUTDOWN);
      sleep(5);
    }
    if (!pid) {
      /* child */

        if ((ttyfd =open("/dev/console", O_RDWR)) != -1) {
#ifdef TIOCSCTTY 
          ioctl(ttyfd, TIOCSCTTY, (char *)0);
#endif
          dup2(ttyfd, 0);
          if (ttyfd > 2) close(ttyfd);
        } else {
          log_warn(LOG_PREFIX, "unable to open /dev/console: ", 0);
        }
      //setsid();

      sig_unblock(SIGALRM);
      sig_unblock(SIGCHLD);
      sig_default(SIGCHLD);
      sig_unblock(SIGCONT);
      sig_default(SIGCONT);
      sig_unblock(SIGHUP);
      sig_unblock(SIGINT);
      sig_default(SIGINT);
      sig_unblock(SIGPIPE);
      sig_unblock(SIGTERM);
            
      log_info(LOG_PREFIX, "execute ", action);
      prog[0] = action;
      prog[1] = 0;
      execve(*prog, (char *const *)prog, envp);
      log_error(LOG_PREFIX, "unable to execute ", action);
      _exit(1);
    }

    if (wait_pid(&wstat, pid) == -1)
      log_warn(LOG_PREFIX, "wait_pid...", 0);
}


/*
 * main entry
 */
int main(int argc, char *argv[], char * const *envp) {
  int ttyfd;
  char *progname = argv[0];
  char *flagname = HALT;

  if (getpid() != 1) {
    unlink(HALT);
    unlink(POWEROFF);
    unlink(REBOOT);

    int i = str_len(progname);
    while (i > 0 && progname[i-1] != '/') {
      i--;
    }
    progname += i;

    if (str_equal(progname, "poweroff"))
      flagname = POWEROFF;
    if (str_equal(progname, "reboot"))
      flagname = REBOOT;

    int fd = creat(flagname, 0600);
    if (fd < 0 ) {
        log_error(LOG_PREFIX, "unable to create " , flagname);
        _exit(1);
    }
    close(fd);
    kill(1, SIGCONT);
    _exit(0);
  }

  setsid();

  sig_block(SIGALRM);
  sig_block(SIGCHLD);
  sig_block(SIGCONT);
  sig_block(SIGHUP);
  sig_block(SIGINT);
  sig_block(SIGPIPE);
  sig_block(SIGTERM);

  /* console */
  if ((ttyfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) != -1) {
    dup2(ttyfd, 0); dup2(ttyfd, 1); dup2(ttyfd, 2);
    if (ttyfd > 2) close(ttyfd);
  }

#ifdef RB_DISABLE_CAD
  /* activate ctrlaltdel handling, glibc, dietlibc */
  if (RB_DISABLE_CAD == 0) reboot(0);
#endif

  execute(STARTUP, envp);

  log_info(LOG_PREFIX, "running services ...", 0);
  services(envp);

  execute(SHUTDOWN, envp);
  log_info(LOG_PREFIX, "shutdown in progress ...", 0);
  
  /* reget stderr */
  if ((ttyfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) != -1) {
    dup2(ttyfd, 2);
    if (ttyfd > 2) close(ttyfd);
  }

  /* fallthrough stage 3 */
  log_info(LOG_PREFIX, "sending KILL signal to all processes ...", 0);
  kill(-1, SIGKILL);

  sync();
  int um = umount("/");
  if ( um == 0) {
    log_info(LOG_PREFIX, "umount root fs", 0);
  } else {
    log_warn(LOG_PREFIX, "umount root error", 0);
  }
  log_info(LOG_PREFIX, "system is down.", 0);
  reboot(reboot_mode);
  /* not reached */
}

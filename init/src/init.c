#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "sig.h"
#include "str.h"
#include "strerr.h"
#include "error.h"
#include "iopause.h"
#include "ndelay.h"
#include "wait.h"
#include "open.h"

#define HALT     "/run/shutdown.halt"
#define POWEROFF "/run/shutdown.poweroff"
#define REBOOT   "/run/shutdown.reboot"

#define DEBUG

#define INFO "[ INIT ] "
#define WARNING "[ INIT ] warning: "
#define FATAL "[ INIT ] fatal: "

const char * const stage[3] ={
  "/etc/startup",
  "/sbin/runsvdir",
  "/etc/shutdown" };

int selfpipe[2];
int sigc =0;
int sigi =0;

/*
 * signal handlers
 */
void (*sig_defaulthandler)() = SIG_DFL;
void (*sig_ignorehandler)() = SIG_IGN;

void sig_cont_handler(void) {
  sigc++;
  write(selfpipe[1], "", 1);
}

void sig_int_handler(void) {
  sigi++;
  write(selfpipe[1], "", 1);
}

void sig_child_handler(void) {
  write(selfpipe[1], "", 1);
}

/*
 * main entry
 */
int main(int argc, char *argv[], char * const *envp) {
  const char * prog[2];
  int pid;
  int wstat;
  int st;
  iopause_fd x;
  char ch;
  int ttyfd;
  char *flagname = HALT;
  char *progname = argv[0];
  int reboot_mode = 0;



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
    if (fd < 0 )
        strerr_die4x(0, FATAL, "unable to create " , flagname, "\n");
    close(fd);
    kill(1, SIGCONT);
    _exit(0);
  }

  setsid();

  sig_block(SIGALRM);
  sig_block(SIGCHLD);
  sig_catch(SIGCHLD, sig_child_handler);
  sig_block(SIGCONT);
  sig_catch(SIGCONT, sig_cont_handler);
  sig_block(SIGHUP);
  sig_block(SIGINT);
  sig_catch(SIGINT, sig_int_handler);
  sig_block(SIGPIPE);
  sig_block(SIGTERM);

  /* console */
  if ((ttyfd =open_write("/dev/console")) != -1) {
    dup2(ttyfd, 0); dup2(ttyfd, 1); dup2(ttyfd, 2);
    if (ttyfd > 2) close(ttyfd);
  }

  /* create selfpipe */
  while (pipe(selfpipe) == -1) {
    strerr_warn2(FATAL, "unable to create selfpipe, pausing: ", &strerr_sys);
    sleep(5);
  }
  fcntl(selfpipe[0], F_SETFD, 1);
  fcntl(selfpipe[1], F_SETFD, 1);
  ndelay_on(selfpipe[0]);
  ndelay_on(selfpipe[1]);

#ifdef RB_DISABLE_CAD
  /* activate ctrlaltdel handling, glibc, dietlibc */
  if (RB_DISABLE_CAD == 0) reboot(0);
#endif

  strerr_warn2(INFO, "booting in progress ...", 0);

  /* init loop */
  for (st =0; st < 3; st++) {
    while ((pid =fork()) == -1) {
      strerr_warn4(FATAL, "unable to fork for \"", stage[st], "\" pausing: ",
                   &strerr_sys);
      sleep(5);
    }
    if (!pid) {
      /* child */
      prog[0] =stage[st];
      prog[1] =0;

      /* stage 1 gets full control of console */
      if (st == 0) {
        if ((ttyfd =open("/dev/console", O_RDWR)) != -1) {
#ifdef TIOCSCTTY 
          ioctl(ttyfd, TIOCSCTTY, (char *)0);
#endif
          dup2(ttyfd, 0);
          if (ttyfd > 2) close(ttyfd);
        }
        else
          strerr_warn2(WARNING, "unable to open /dev/console: ", &strerr_sys);
      }
      else
        setsid();

      sig_unblock(SIGALRM);
      sig_unblock(SIGCHLD);
      sig_uncatch(SIGCHLD);
      sig_unblock(SIGCONT);
      sig_ignore(SIGCONT);
      sig_unblock(SIGHUP);
      sig_unblock(SIGINT);
      sig_uncatch(SIGINT);
      sig_unblock(SIGPIPE);
      sig_unblock(SIGTERM);
            
      strerr_warn3(INFO, "enter stage: ", stage[st], 0);
      execve(*prog, (char *const *)prog, envp);
      strerr_die4sys(0, FATAL, "unable to start child: ", stage[st], ": ");
    }

    x.fd =selfpipe[0];
    x.events =IOPAUSE_READ;
    for (;;) {
      int child;

      sig_unblock(SIGCHLD);
      sig_unblock(SIGCONT);
      sig_unblock(SIGINT);
      // poll with 14 s timeout
      poll(&x, 1, 14000);
      sig_block(SIGCONT);
      sig_block(SIGCHLD);
      sig_block(SIGINT);
      
      while (read(selfpipe[0], &ch, 1) == 1) {}
      while ((child =wait_nohang(&wstat)) > 0)
        if (child == pid) break;
      if (child == -1) {
        strerr_warn2(WARNING, "wait_nohang, pausing: ", &strerr_sys);
        sleep(5);
      }

      /* reget stderr */
      if ((ttyfd =open_write("/dev/console")) != -1) {
        dup2(ttyfd, 2);
        if (ttyfd > 2) close(ttyfd);
      }

      if (child == pid) {
        if (wait_exitcode(wstat) != 0) {
          if (wait_crashed(wstat))
            strerr_warn3(WARNING, "child crashed: ", stage[st], 0);
          else
            strerr_warn3(WARNING, "child failed: ", stage[st], 0);
          if (st == 0)
            /* this is stage 1 */
            if (wait_crashed(wstat) || (wait_exitcode(wstat) == 100)) {
              strerr_warn3(INFO, "leave stage: ", stage[st], 0);
              strerr_warn2(WARNING, "skipping stage 2...", 0);
              st++;
              break;
            }
          if (st == 1)
            /* this is stage 2 */
            if (wait_crashed(wstat) || (wait_exitcode(wstat) == 111)) {
              strerr_warn2(WARNING, "killing all processes in stage 2...", 0);
              kill(-pid, 9);
              sleep(5);
              strerr_warn2(WARNING, "restarting.", 0);
              st--;
              break;
            }
        }
        strerr_warn3(INFO, "leave stage: ", stage[st], 0);
        break;
      }
      if (child != 0) {
        /* collect terminated children */
        write(selfpipe[1], "", 1);
        continue;
      }

      /* sig? */
      if (!sigc  && !sigi) {
        continue;
      }
      if (st != 1) {
        strerr_warn2(WARNING, "signals only work in stage 2.", 0);
        sigc = sigi = 0;
        continue;
      }
      if (sigi) {
        strerr_warn2(INFO, "ctrl-alt-del request...", 0);
        sigi = 0;
        sigc++;
      }
      if (sigc) {
        /* choose reboot mode */
        reboot_mode = RB_AUTOBOOT;
        struct stat rstat;
        if (stat(POWEROFF, &rstat) == 0)
          reboot_mode = RB_POWER_OFF;
        if (stat(HALT, &rstat) == 0)
          reboot_mode = RB_HALT_SYSTEM;

        /* kill stage 2 */
#ifdef DEBUG
        strerr_warn2(WARNING, "sending sigterm...", 0);
#endif
        kill(pid, SIGTERM);
        int i = 0;
        while (i < 5) {
          if ((child =wait_nohang(&wstat)) == pid) {
#ifdef DEBUG
            strerr_warn2(WARNING, "stage 2 terminated.", 0);
#endif
            pid =0;
            break;
          }
          if (child) continue;
          if (child == -1) 
            strerr_warn2(WARNING, "wait_nohang: ", &strerr_sys);
#ifdef DEBUG
          strerr_warn2(WARNING, "waiting...", 0);
#endif
          sleep(1);
          i++;
        }
        if (pid) {
          /* still there */
          strerr_warn2(WARNING,
                       "stage 2 not terminated, sending sigkill...", 0);
          kill(pid, 9);
          if (wait_pid(&wstat, pid) == -1)
            strerr_warn2(WARNING, "wait_pid: ", &strerr_sys);
        }
        sigc =0;
        strerr_warn3(INFO, "leave stage: ", stage[st], 0);

        /* enter stage 3 */
        break;
      }
      sigc = sigi = 0;
#ifdef DEBUG
      strerr_warn2(WARNING, "no request.", 0);
#endif
    }
  }

  /* reget stderr */
  if ((ttyfd =open_write("/dev/console")) != -1) {
    dup2(ttyfd, 2);
    if (ttyfd > 2) close(ttyfd);
  }

  /* fallthrough stage 3 */
  strerr_warn2(INFO, "sending KILL signal to all processes ...", 0);
  kill(-1, SIGKILL);

  strerr_warn2(INFO, "shutdown in progress ...", 0);
  sync();
  int um = umount("/");
  if ( um == 0) {
    strerr_warn2(INFO, "umount root fs", 0);
  } else {
    strerr_warn2(WARNING, "umount root error", 0);
  }
  strerr_warn2(INFO, "system is down.", 0);
  reboot(reboot_mode);

  /* not reached */
  strerr_die2x(0, INFO, "exit.");
  return(0);
}


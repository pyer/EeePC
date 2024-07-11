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

#define HALT     "/run/shutdown.halt"
#define POWEROFF "/run/shutdown.poweroff"
#define REBOOT   "/run/shutdown.reboot"

#define LOG_PREFIX "init"
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
  sig_catch(SIGCHLD, sig_child_handler);
  sig_block(SIGCONT);
  sig_catch(SIGCONT, sig_cont_handler);
  sig_block(SIGHUP);
  sig_block(SIGINT);
  sig_catch(SIGINT, sig_int_handler);
  sig_block(SIGPIPE);
  sig_block(SIGTERM);

  /* console */
  if ((ttyfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) != -1) {
    dup2(ttyfd, 0); dup2(ttyfd, 1); dup2(ttyfd, 2);
    if (ttyfd > 2) close(ttyfd);
  }

  /* create selfpipe */
  while (pipe(selfpipe) == -1) {
    log_warn(LOG_PREFIX, "unable to create selfpipe, pausing", 0);
    sleep(5);
  }
  fcntl(selfpipe[0], F_SETFD, 1);
  fcntl(selfpipe[1], F_SETFD, 1);
  fcntl(selfpipe[0], F_SETFL, fcntl(selfpipe[0], F_GETFL, 0) | O_NONBLOCK);
  fcntl(selfpipe[1], F_SETFL, fcntl(selfpipe[1], F_GETFL, 0) | O_NONBLOCK);

#ifdef RB_DISABLE_CAD
  /* activate ctrlaltdel handling, glibc, dietlibc */
  if (RB_DISABLE_CAD == 0) reboot(0);
#endif

  log_info(LOG_PREFIX, "booting in progress ...", 0);

  /* init loop */
  for (st =0; st < 3; st++) {
    while ((pid =fork()) == -1) {
      log_warn(LOG_PREFIX, "unable to fork for ", stage[st]);
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
        } else {
          log_warn(LOG_PREFIX, "unable to open /dev/console: ", 0);
        }
      } else {
        setsid();
      }

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
            
      log_info(LOG_PREFIX, "enter stage: ", stage[st]);
      execve(*prog, (char *const *)prog, envp);
      log_error(LOG_PREFIX, "unable to start child: ", stage[st]);
      _exit(1);
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
        log_warn(LOG_PREFIX, "wait_nohang, pausing: ", 0);
        sleep(5);
      }

      /* reget stderr */
      if ((ttyfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) != -1) {
        dup2(ttyfd, 2);
        if (ttyfd > 2) close(ttyfd);
      }

      if (child == pid) {
        if (wait_exitcode(wstat) != 0) {
          if (wait_crashed(wstat))
            log_warn(LOG_PREFIX, "child crashed: ", stage[st]);
          else
            log_warn(LOG_PREFIX, "child failed: ", stage[st]);
          if (st == 0)
            /* this is stage 1 */
            if (wait_crashed(wstat) || (wait_exitcode(wstat) == 100)) {
              log_info(LOG_PREFIX, "leave stage: ", stage[st]);
              log_warn(LOG_PREFIX, "skipping stage 2...", 0);
              st++;
              break;
            }
          if (st == 1)
            /* this is stage 2 */
            if (wait_crashed(wstat) || (wait_exitcode(wstat) == 111)) {
              log_info(LOG_PREFIX, "killing all processes in stage 2...", 0);
              kill(-pid, 9);
              sleep(5);
              log_info(LOG_PREFIX, "restarting.", 0);
              st--;
              break;
            }
        }
        log_info(LOG_PREFIX, "leave stage: ", stage[st]);
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
        log_info(LOG_PREFIX, "signals only work in stage 2.", 0);
        sigc = sigi = 0;
        continue;
      }
      if (sigi) {
        log_info(LOG_PREFIX, "ctrl-alt-del request...", 0);
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
        log_info(LOG_PREFIX, "sending sigterm...", 0);
#endif
        kill(pid, SIGTERM);
        int i = 0;
        while (i < 5) {
          if ((child =wait_nohang(&wstat)) == pid) {
#ifdef DEBUG
            log_info(LOG_PREFIX, "stage 2 terminated.", 0);
#endif
            pid =0;
            break;
          }
          if (child) continue;
          if (child == -1) 
            log_warn(LOG_PREFIX, "wait_nohang: ", 0);
#ifdef DEBUG
          log_info(LOG_PREFIX, "waiting...", 0);
#endif
          sleep(1);
          i++;
        }
        if (pid) {
          /* still there */
          log_info(LOG_PREFIX, "stage 2 not terminated, sending sigkill...", 0);
          kill(pid, 9);
          if (wait_pid(&wstat, pid) == -1)
            log_warn(LOG_PREFIX, "wait_pid...", 0);
        }
        sigc =0;
        log_info(LOG_PREFIX, "leave stage: ", stage[st]);

        /* enter stage 3 */
        break;
      }
      sigc = sigi = 0;
#ifdef DEBUG
      log_info(LOG_PREFIX, "no request.", 0);
#endif
    }
  }

  /* reget stderr */
  if ((ttyfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) != -1) {
    dup2(ttyfd, 2);
    if (ttyfd > 2) close(ttyfd);
  }

  /* fallthrough stage 3 */
  log_info(LOG_PREFIX, "sending KILL signal to all processes ...", 0);
  kill(-1, SIGKILL);

  log_info(LOG_PREFIX, "shutdown in progress ...", 0);
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
  return(0);
}


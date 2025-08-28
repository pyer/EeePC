
/* logon.c
 *
 * Usage: logon <tty name>
 *
 * logon replaces getty and needs only one argument, the device name (e.g. /dev/tty1)
 */

#define _GNU_SOURCE 1           /* Needed to get setsid() */

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "log.h"
#define LOG_PREFIX "logon"

/* login program invoked */
static char *loginprog = "/bin/login";

/* error() - output error messages */
static void error(const char *msg1, const char *msg2)
{
  log_error(LOG_PREFIX, msg1, msg2);
//  sleep(5);
  exit(EXIT_FAILURE);
}

/* open_tty - set up tty as standard { input, output, error } */
static void open_tty(char *tty)
{
  struct sigaction sa, sa_old;
  int fd;

  /* Set up new standard input. */
  /* There is always a race between this reset and the call to
     vhangup() that s.o. can use to get access to your tty. */
  if (chown (tty, 0, -1) || chmod (tty, 0600)) {
      error(tty, " :");
  }

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);
  sigaction (SIGHUP, &sa, &sa_old);

  /* vhangup() will replace all open file descriptors in the kernel
     that point to our controlling tty by a dummy that will deny
     further reading/writing to our device. It will also reset the
     tty to sane defaults, so we don't have to modify the tty device
     for sane settings. We also get a SIGHUP/SIGCONT.
   */
  if ((fd = open (tty, O_RDWR, 0)) < 0)
    error("cannot open ", tty);
  if (ioctl (fd, TIOCSCTTY, (void *) 1) == -1)
    error("cannot control ", tty);
  if (!isatty (fd))
    error(tty, "is not a tty");

  if (vhangup ())
    error(tty, " vhangup() failed");

  /* Get rid of the present stdout/stderr. */
  close (2);
  close (1);
  close (0);
  close (fd);
  if ((fd = open (tty, O_RDWR, 0)) != 0)
    error("cannot open ", tty);
  if (ioctl (fd, TIOCSCTTY, (void *) 1) == -1)
    error("cannot control ", tty);
 
  /* Set up stdin/stdout/stderr. */
  if (dup2 (fd, 0) != 0 || dup2 (fd, 1) != 1 || dup2 (fd, 2) != 2)
    error(tty, " dup2() failed");
  if (fd > 2)
    close (fd);
  sigaction (SIGHUP, &sa_old, NULL);
}

/* show /etc/issue content */
static void show_issue(void)
{
  FILE *fd;
  int c;

  putchar('\n');
  fd = fopen("/etc/issue", "r");
  if (fd != NULL) {
    while ((c = getc(fd)) != EOF) {
        putchar(c);
    }
    fclose(fd);
  }
}

static void show_prompt(void)
{
  printf("Login: ");
  fflush(stdout);
}

static void get_logname(char *logname)
{
  char *bp = logname;
  unsigned char c = 1;

  tcflush(0, TCIFLUSH);    /* flush pending input */
  while (c != 0 ) {
    if (read (0, &c, 1) < 1) {
        if (errno == EINTR || errno == EIO || errno == ENOENT)
          exit (EXIT_SUCCESS);
        error("cannot read tty","");
    }
    if (c == '\n' || c == '\r') {
        c = 0;
    }
    *bp++ = c;

    if ((size_t)(bp - logname) > 40 ) {
        *bp = 0;
        error("login name is too long", "");
    }
  }
}

/*
 * MAIN ENTRY
 */
int main (int argc, char **argv)
{
  char logname[42];

  /* argv[1] is the tty name */
  if (argc == 3) {
    open_tty(argv[1]);
    /* Automatic login: argv[2] is the user name */
    execl(loginprog, loginprog, "-f", argv[2], NULL);
  } else if (argc == 2) {
    open_tty(argv[1]);
    show_issue();
    *logname = 0;
    while (*logname == 0) {
      show_prompt();
      get_logname(logname);
    }
    execl(loginprog, loginprog, "--", logname, NULL);
  } else {
    log_error(LOG_PREFIX, "Usage: logon <tty name> [<user name>]", "");
    return 111;
  }
  error("cannot execute ", loginprog);
}


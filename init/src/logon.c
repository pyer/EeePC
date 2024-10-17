
/* logon.c
 *
 * Usage: logon <tty name>
 *
 * logon replaces getty and needs only one argument, the device name (e.g. /dev/tty1)
 */

/*  mingetty.c
 *
 *  Copyright (C) 1996 Florian La Roche  <laroche@redhat.com>
 *  Copyright (C) 2002, 2003 Red Hat, Inc
 *
 *  This getty can only be used as a small console getty. Look at mgetty
 *  for a real modem getty.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE 1           /* Needed to get setsid() */
/*
#include <termios.h>
#include <utmp.h>
#include <getopt.h>
#include <sys/param.h>
#include <time.h>
*/


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <signal.h>
#include <fcntl.h>
#include <stdarg.h>

/* name of this program (argv[0]) */
static char *progname;
/* on which tty line are we sitting? (e.g. /dev/tty1) */
static char *tty;
/* login program invoked */
static char *loginprog = "/bin/login";

/* error() - output error messages */
static void error (const char *fmt, ...)
{
  va_list va_alist;

  va_start (va_alist, fmt);
  openlog (progname, LOG_PID, LOG_AUTH);
  vsyslog (LOG_ERR, fmt, va_alist);
  /* no need, we exit anyway: closelog (); */
  va_end (va_alist);
  sleep (5);
  exit (EXIT_FAILURE);
}

/* open_tty - set up tty as standard { input, output, error } */
static void open_tty (void)
{
  struct sigaction sa, sa_old;
  char buf[40];
  int fd;

  /* Reset permissions on the console device */
  if ((strncmp(tty, "/dev/tty", 8) == 0) && (isdigit(tty[8]))) {
    strcpy (buf, "/dev/vcs");
    strcat (buf, &tty[8]);
    if (chown (buf, 0, 3) || chmod (buf, 0600))
      if (errno != ENOENT)
        error ("%s: %s", buf, strerror(errno));

    strcpy (buf, "/dev/vcsa");
    strcat (buf, &tty[8]);
    if (chown (buf, 0, 3) || chmod (buf, 0600))
      if (errno != ENOENT)
        error ("%s: %s", buf, strerror(errno));
  }

  /* Set up new standard input. */
  strncpy (buf, tty, sizeof(buf)-1);
  buf[sizeof(buf)-1] = '\0';
  /* There is always a race between this reset and the call to
     vhangup() that s.o. can use to get access to your tty. */
  if (chown (buf, 0, 0) || chmod (buf, 0600))
    if (errno != EROFS)
      error ("%s: %s", tty, strerror (errno));

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
  if ((fd = open (buf, O_RDWR, 0)) < 0)
    error ("%s: cannot open tty: %s", tty, strerror (errno));
  if (ioctl (fd, TIOCSCTTY, (void *) 1) == -1)
    error ("%s: no controlling tty: %s", tty, strerror (errno));
  if (!isatty (fd))
    error ("%s: not a tty", tty);

  if (vhangup ())
      error ("%s: vhangup() failed", tty);
    /* Get rid of the present stdout/stderr. */
    close (2);
    close (1);
    close (0);
    close (fd);
  if ((fd = open (buf, O_RDWR, 0)) != 0)
      error ("%s: cannot open tty: %s", tty, strerror (errno));
  if (ioctl (fd, TIOCSCTTY, (void *) 1) == -1)
      error ("%s: no controlling tty: %s", tty, strerror (errno));
 
  /* Set up stdin/stdout/stderr. */
  if (dup2 (fd, 0) != 0 || dup2 (fd, 1) != 1 || dup2 (fd, 2) != 2)
    error ("%s: dup2(): %s", tty, strerror (errno));
  if (fd > 2)
    close (fd);
  sigaction (SIGHUP, &sa_old, NULL);
}

/* do_prompt - show login prompt, optionally preceded by /etc/issue contents */
static void do_prompt (void)
{
  FILE *fd;
  int c;

  putchar ('\n');
  fd = fopen ("/etc/issue", "r");
  if (fd != NULL) {
    while ((c = getc (fd)) != EOF) {
        putchar (c);
    }
    fclose (fd);
  }
  printf ("Login: ");
  fflush (stdout);
}

static char *get_logname (void)
{
  static char logname[40];
  char *bp;
  unsigned char c;

  // tcflush (0, TCIFLUSH);    /* flush pending input */
  for (*logname = 0; *logname == 0;) {
    do_prompt();
    for (bp = logname;;) {
      if (read (0, &c, 1) < 1) {
        if (errno == EINTR || errno == EIO
          || errno == ENOENT)
          exit (EXIT_SUCCESS);
        error ("%s: read: %s", tty, strerror (errno));
      }
      if (c == '\n' || c == '\r') {
        *bp = 0;
        break;
      } else if (!isprint (c))
        error ("%s: invalid character 0x%x in login name", tty, c);
      else if ((size_t)(bp - logname) >= sizeof (logname) - 1)
        error ("%s: too long login name", tty);
      else
        *bp++ = c;
    }
  }
  return logname;
}

/*
 * MAIN ENTRY
 */
int main (int argc, char **argv)
{
	char *logname;
	progname = argv[0];
//  putenv ("TERM=linux");

  if (argc != 2) {
    error("Usage: %s <tty name>", progname);
    return 111;
  }

  tty = argv[1];
  if (!tty) {
    error("tty not found");
    return 111;
  }

  /* ignore leading "/dev/" */
  if (strncmp (tty, "/dev/", 5) == 0) {
    tty += 5;
  }

//  update_utmp ();
  open_tty();
  while ((logname = get_logname ()) == 0)
      /* do nothing */ ;

  execl(loginprog, loginprog, "--", logname, NULL);
  error("%s: can't exec %s: %s", tty, loginprog, strerror(errno));
  sleep(5);
  exit(EXIT_FAILURE);
}


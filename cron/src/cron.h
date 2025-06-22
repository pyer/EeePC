/*
 * Copyright (c) 1988,1990,1993,1994,2021 by Paul Vixie ("VIXIE")
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND VIXIE DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL VIXIE BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* cron.h - header for vixie's cron
 *
 * $Id: cron.h,v 1.6 2004/01/23 18:56:42 vixie Exp $
 *
 * vix 14nov88 [rest of log is in RCS]
 * vix 14jan87 [0 or 7 can be sunday; thanks, mwm@berkeley]
 * vix 30dec86 [written]
 */

#define CRON_VERSION "5.0"
#define CRON_PID     "/run/cron.pid"
#define CRON_TAB	   "/etc/crontab"


/* reorder these #include's at your peril */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <bitstring.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define DIR_T	struct dirent
#define WAIT_T	int
#define SIG_T	sig_t
#define TIME_T	time_t
#define PID_T	pid_t

/*
#ifndef TZNAME_ALREADY_DEFINED
extern char *tzname[2];
#endif
#define TZONE(tm) tzname[(tm).tm_isdst]
*/

#include "macros.h"
#include "structs.h"
#include "funcs.h"
#include "globals.h"
#include "log.h"

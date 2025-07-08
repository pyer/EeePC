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

#include "cron.h"

static bool crontab_is_modified(struct timespec cron, struct timespec db) {
	if (cron.tv_sec > db.tv_sec)
		return TRUE;
	else if (cron.tv_sec < db.tv_sec)
		return FALSE;
	return cron.tv_nsec > db.tv_nsec;
}


/* something's different.  make a new database, moving unchanged
 * elements from the old database, reloading elements that have
 * actually changed.  Whatever is left in the old database when
 * we're done is chaff -- crontabs that disappeared.
 */
void clear_database(cron_db *db) {
  entry *e, *ne = NULL;
    // whatever's left in the old database is now junk.
    Debug(DLOAD, ("unlinking old database\n"))
    for (e = db->entrypoint;  e != NULL;  e = ne) {
      ne = e->next;
      free_entry(e);
    }
    db->entrypoint = NULL;
}

static void process_crontab(const char *tabname, struct stat *statbuf, cron_db *db)
{
  int crontab_fd = OK - 1;
  FILE *file;
  entry *e;
  int ch;

  Debug(DLOAD, ("Process crontab\n"))
  if ((crontab_fd = open(tabname, O_RDONLY|O_NONBLOCK|O_NOFOLLOW, 0)) < OK) {
    /* crontab not accessible?
     */
    log_it("INIT", getpid(), "CAN'T OPEN", tabname);
    goto next_crontab;
  }

  if (fstat(crontab_fd, statbuf) < OK) {
    log_it("INIT", getpid(), "FSTAT FAILED", tabname);
    goto next_crontab;
  }
  if (!S_ISREG(statbuf->st_mode)) {
    log_it("INIT", getpid(), "NOT REGULAR", tabname);
    goto next_crontab;
  }
  if (statbuf->st_nlink != 1) {
    log_it("INIT", getpid(), "BAD LINK COUNT", tabname);
    goto next_crontab;
  }

  log_it("INIT", getpid(), "LOAD", tabname);

  file = fdopen(crontab_fd, "r");
  if (file == NULL) {
    perror("fdopen on crontab_fd in load_user");
  } else {
    do {
      ch = skip_comments(file);
      if( ch != EOF) {
        e = load_entry(file);
        if (e) {
          e->next = db->entrypoint;
          db->entrypoint = e;
        }
      }
    } while( ch != EOF);
    fclose(file);
  }

 next_crontab:
  if (crontab_fd >= OK) {
    Debug(DLOAD, (" [done]\n"))
    close(crontab_fd);
  }
}


void load_database(cron_db *db) {
  struct stat syscron_stat;

  Debug(DLOAD, ("[%ld] load_database()\n", (long)getpid()))

  /* track system crontab file
   */
  if (stat(CRON_TAB, &syscron_stat) < OK) {
    syscron_stat.st_mtim.tv_sec  = 0;
    syscron_stat.st_mtim.tv_nsec = 0;
  }

  Debug(DLOAD, ("STAT %s: %ld , database: %ld\n", CRON_TAB, syscron_stat.st_mtim.tv_sec, db->mtim.tv_sec))
  if (crontab_is_modified(syscron_stat.st_mtim, db->mtim)) {
    clear_database(db);
    process_crontab(CRON_TAB, &syscron_stat, db);
    db->mtim.tv_sec  = syscron_stat.st_mtim.tv_sec;
    db->mtim.tv_nsec = syscron_stat.st_mtim.tv_nsec;
  }

  Debug(DLOAD, ("load_database is done\n"))
}


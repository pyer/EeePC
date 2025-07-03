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


/* something's different.  make a new database, moving unchanged
 * elements from the old database, reloading elements that have
 * actually changed.  Whatever is left in the old database when
 * we're done is chaff -- crontabs that disappeared.
 */
void clear_database(cron_db *db) {
  entry *e, *ne;
    // whatever's left in the old database is now junk.
    Debug(DLOAD, ("unlinking old database:\n"))
    for (e = db->entrypoint;  e != NULL;  e = ne) {
      Debug(DLOAD, ("\t%s\n", e->name))
      ne = e->next;
      free_entry(e);
    }
    db->entrypoint = NULL;
}

static void process_crontab(const char *tabname, struct stat *statbuf, cron_db *new_db)
{
  char *fname = "root";
  struct passwd *pw = NULL;
  int crontab_fd = OK - 1;
  FILE *file;
  entry *e;
  int st;
  int ch;
  char **envp;

  if ((crontab_fd = open(tabname, O_RDONLY|O_NONBLOCK|O_NOFOLLOW, 0)) < OK) {
    /* crontab not accessible?
     */
    log_it(fname, getpid(), "CAN'T OPEN", tabname);
    goto next_crontab;
  }

  if (fstat(crontab_fd, statbuf) < OK) {
    log_it(fname, getpid(), "FSTAT FAILED", tabname);
    goto next_crontab;
  }
  if (!S_ISREG(statbuf->st_mode)) {
    log_it(fname, getpid(), "NOT REGULAR", tabname);
    goto next_crontab;
  }
  if (statbuf->st_nlink != 1) {
    log_it(fname, getpid(), "BAD LINK COUNT", tabname);
    goto next_crontab;
  }

  log_it(fname, getpid(), "LOAD", tabname);

  /* init environment.  this will be copied/augmented for each entry.
   */

  if ((envp = env_init()) == NULL) {
    goto next_crontab;
  }

  if (!(file = fdopen(crontab_fd, "r"))) {
    perror("fdopen on crontab_fd in load_user");
    goto next_crontab;
  }

  do {
      ch = skip_comments(file);
      if (ch>='A' && ch<='Z') {
        st = load_env(file);
        if (st == FALSE)
          log_it(fname, getpid(), "ENV", "error");
        else
          log_it(fname, getpid(), "ENV", "ok");
      } else {
        e = load_entry(file, envp);
        if (e) {
          e->next = new_db->entrypoint;
          new_db->entrypoint = e;
        }
      }
  } while( ch != EOF);

  fclose(file);
  env_free(envp);

 next_crontab:
  if (crontab_fd >= OK) {
    Debug(DLOAD, (" [done]\n"))
    close(crontab_fd);
  }
}


void load_database(cron_db *db) { struct stat statbuf, syscron_stat;

  Debug(DLOAD, ("[%ld] load_database()\n", (long)getpid()))

  /* track system crontab file
   */
  if (stat(CRON_TAB, &syscron_stat) < OK)
    syscron_stat.st_mtim = ts_zero;

  Debug(DLOAD, ("STAT %s: %ld , database: %ld\n", CRON_TAB, syscron_stat.st_mtim.tv_sec, db->mtim.tv_sec))
//  if (!TEQUAL(syscron_stat.st_mtim, ts_zero))
  if (syscron_stat.st_mtim.tv_sec > db->mtim.tv_sec) {
    clear_database(db);
    process_crontab(CRON_TAB, &syscron_stat, db);
    db->mtim.tv_sec = syscron_stat.st_mtim.tv_sec;
  }

  Debug(DLOAD, ("load_database is done\n"))
}


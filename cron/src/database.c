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

#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Id: database.c,v 1.7 2004/01/23 18:56:42 vixie Exp $";
#endif

/* vix 26jan87 [RCS has the log]
 */

#include "cron.h"

#define TMAX(a,b) (is_greater_than(a,b)?(a):(b))
#define TEQUAL(a,b) (a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec)

static bool
is_greater_than(struct timespec left, struct timespec right) {
	if (left.tv_sec > right.tv_sec)
		return TRUE;
	else if (left.tv_sec < right.tv_sec)
		return FALSE;
	return left.tv_nsec > right.tv_nsec;
}


static	void		process_crontab(
					const char *, struct stat *,
					cron_db *, cron_db *);

void
load_database(cron_db *old_db) {
	struct stat statbuf, syscron_stat;
	cron_db new_db;
	user *u, *nu;

	Debug(DLOAD, ("[%ld] load_database()\n", (long)getpid()))

	/* track system crontab file
	 */
	if (stat(CRON_TAB, &syscron_stat) < OK)
		syscron_stat.st_mtim = ts_zero;

	/* something's different.  make a new database, moving unchanged
	 * elements from the old database, reloading elements that have
	 * actually changed.  Whatever is left in the old database when
	 * we're done is chaff -- crontabs that disappeared.
	 */
	new_db.mtim = syscron_stat.st_mtim;
	new_db.head = new_db.tail = NULL;

	if (!TEQUAL(syscron_stat.st_mtim, ts_zero))
		process_crontab(CRON_TAB, &syscron_stat, &new_db, old_db);

	/* whatever's left in the old database is now junk.
	 */
	Debug(DLOAD, ("unlinking old database:\n"))
	for (u = old_db->head;  u != NULL;  u = nu) {
		Debug(DLOAD, ("\t%s\n", u->name))
		nu = u->next;
		unlink_user(old_db, u);
		free_user(u);
	}

	/* overwrite the database control block with the new one.
	 */
	*old_db = new_db;
	Debug(DLOAD, ("load_database is done\n"))
}

void
link_user(cron_db *db, user *u) {
	if (db->head == NULL)
		db->head = u;
	if (db->tail)
		db->tail->next = u;
	u->prev = db->tail;
	u->next = NULL;
	db->tail = u;
}

void
unlink_user(cron_db *db, user *u) {
	if (u->prev == NULL)
		db->head = u->next;
	else
		u->prev->next = u->next;

	if (u->next == NULL)
		db->tail = u->prev;
	else
		u->next->prev = u->prev;
}

user *
find_user(cron_db *db, const char *name) {
	user *u = NULL;

	for (u = db->head;  u != NULL;  u = u->next)
		if (strcmp(u->name, name) == 0)
			break;
  if (u == NULL) {
		log_it("NULL", getpid(), "CAN'T FIND USER", name);
  } else {
		log_it(u->name, getpid(), "FIND USER", name);
  }

	return (u);
}

static void
process_crontab(const char *tabname,
		struct stat *statbuf, cron_db *new_db, cron_db *old_db)
{
  char *fname = "root";
	struct passwd *pw = NULL;
	int crontab_fd = OK - 1;
	user *u;

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

	u = find_user(old_db, fname);
	if (u != NULL) {
		/* if crontab has not changed since we last read it
		 * in, then we can just use our existing entry.
		 */
		if (TEQUAL(u->mtim, statbuf->st_mtim)) {
			Debug(DLOAD, (" [no change, using old data]"))
			unlink_user(old_db, u);
			link_user(new_db, u);
			goto next_crontab;
		}

		/* before we fall through to the code that will reload
		 * the user, let's deallocate and unlink the user in
		 * the old database.  This is more a point of memory
		 * efficiency than anything else, since all leftover
		 * users will be deleted from the old database when
		 * we finish with the crontab...
		 */
		Debug(DLOAD, (" [delete old data]"))
		unlink_user(old_db, u);
		free_user(u);
		log_it(fname, getpid(), "RELOAD", tabname);
	}
	u = load_user(crontab_fd, pw, fname);
	if (u != NULL) {
		u->mtim = statbuf->st_mtim;
		link_user(new_db, u);
	}

 next_crontab:
	if (crontab_fd >= OK) {
		Debug(DLOAD, (" [done]\n"))
		close(crontab_fd);
	}
}

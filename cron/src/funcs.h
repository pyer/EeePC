/*
 * $Id: funcs.h,v 1.9 2004/01/23 18:56:42 vixie Exp $
 */

/*
 * Copyright (c) 2021 by Paul Vixie ("VIXIE")
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

/* Notes:
 *  This file has to be included by cron.h after data structure defs.
 *  We should reorg this into sections by module.
 */

void    set_cron_uid(void),
    set_cron_cwd(void),
    load_database(cron_db *),
    open_logfile(void),
    sigpipe_func(void),
    job_add(const entry *),
    do_command(const entry *),
    unget_char(int, FILE *),
    free_entry(entry *),
    acquire_daemonlock(int);

int    job_runqueue(void),
    set_debug_flags(const char *),
    get_char(FILE *),
    get_string(char *, int, FILE *, char *),
    cron_pclose(FILE *),
    glue_strings(char *, size_t, const char *, const char *, char),
    strcmp_until(const char *, const char *, char),
    allowed(const char *, const char *, const char *),
    skip_comments(FILE *),
    strdtb(char *),
    strcountstr(const char *, const char *);

char *mkprints(unsigned char *, unsigned int);

entry    *load_entry(FILE *);

FILE    *cron_popen(char *, char *, struct passwd *);

struct passwd  *pw_dup(const struct passwd *);

long    get_gmtoff(time_t *, struct tm *);

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

static int  get_list(bitstr_t *, int, int, const char *[], int, FILE *),
    get_range(bitstr_t *, int, int, const char *[], int, FILE *),
    get_number(int *, int, const char *[], int, FILE *, const char *),
    set_element(bitstr_t *, int, int, int),
    set_range(bitstr_t *, int, int, int, int, int);

void free_entry(entry *e) {
  if (e->name)
    free(e->name);
  if (e->home)
    free(e->home);
  if (e->shell)
    free(e->shell);
  if (e->cmd)
    free(e->cmd);
  free(e);
}

/* return NULL if eof or syntax error occurs;
 * otherwise return a pointer to a new entry.
 */
entry * load_entry(FILE *file)
{
  /* this function reads one crontab entry -- the next -- from a file.
   * it skips any leading blank lines, ignores comments, and returns
   * NULL if for any reason the entry can't be read and parsed.
   *
   * syntax (/etc/crontab):
   *  minutes hours doms months dows USERNAME cmd\n
   */

  entry *e;
  int ch;
  char cmd[MAX_COMMAND];

  Debug(DPARS, ("load_entry()... about to eat comments\n"))
  skip_comments(file);

  ch = get_char(file);
  if (ch == EOF)
    return (NULL);

  /* ch is now the first useful character of a useful line.
   * it may be an @special or it may be the first character
   * of a list of minutes.
   */

  Debug(DPARS, ("load_entry()...\n"))
  e = (entry *) calloc(sizeof(entry), sizeof(char));
  Debug(DPARS, ("load_entry()... %p\n",e))

  if (ch == '@') {
    /* all of these should be flagged and load-limited; i.e.,
     * instead of @hourly meaning "0 * * * *" it should mean
     * "close to the front of every hour but not 'til the
     * system load is low".  Problems are: how do you know
     * what "low" means? (save me from /etc/cron.conf!) and:
     * how to guarantee low variance (how low is low?), which
     * means how to we run roughly every hour -- seems like
     * we need to keep a history or let the first hour set
     * the schedule, which means we aren't load-limited
     * anymore.  too much for my overloaded brain. (vix, jan90)
     * HINT
     */
    ch = get_string(cmd, MAX_COMMAND, file, " \t\n");
    if (!strcmp("reboot", cmd)) {
      e->flags |= WHEN_REBOOT;
    } else if (!strcmp("yearly", cmd) || !strcmp("annually", cmd)){
      set_element(e->minute, FIRST_MINUTE, LAST_MINUTE,
            FIRST_MINUTE);
      set_element(e->hour, FIRST_HOUR, LAST_HOUR,
            FIRST_HOUR);
      set_element(e->dom, FIRST_DOM, LAST_DOM,
            FIRST_DOM);
      set_element(e->month, FIRST_MONTH, LAST_MONTH,
            FIRST_MONTH);
      set_range(e->dow, FIRST_DOW, LAST_DOW,
          FIRST_DOW, LAST_DOW, 1);
      e->flags |= DOW_STAR;
    } else if (!strcmp("monthly", cmd)) {
      set_element(e->minute, FIRST_MINUTE, LAST_MINUTE,
            FIRST_MINUTE);
      set_element(e->hour, FIRST_HOUR, LAST_HOUR,
            FIRST_HOUR);
      set_element(e->dom, FIRST_DOM, LAST_DOM,
            FIRST_DOM);
      set_range(e->month, FIRST_MONTH, LAST_MONTH,
          FIRST_MONTH, LAST_MONTH, 1);
      set_range(e->dow, FIRST_DOW, LAST_DOW,
          FIRST_DOW, LAST_DOW, 1);
      e->flags |= DOW_STAR;
    } else if (!strcmp("weekly", cmd)) {
      set_element(e->minute, FIRST_MINUTE, LAST_MINUTE,
            FIRST_MINUTE);
      set_element(e->hour, FIRST_HOUR, LAST_HOUR,
            FIRST_HOUR);
      set_range(e->dom, FIRST_DOM, LAST_DOM,
          FIRST_DOM, LAST_DOM, 1);
      set_range(e->month, FIRST_MONTH, LAST_MONTH,
          FIRST_MONTH, LAST_MONTH, 1);
      set_element(e->dow, FIRST_DOW, LAST_DOW,
            FIRST_DOW);
      e->flags |= DOW_STAR;
    } else if (!strcmp("daily", cmd) || !strcmp("midnight", cmd)) {
      set_element(e->minute, FIRST_MINUTE, LAST_MINUTE,
            FIRST_MINUTE);
      set_element(e->hour, FIRST_HOUR, LAST_HOUR,
            FIRST_HOUR);
      set_range(e->dom, FIRST_DOM, LAST_DOM,
          FIRST_DOM, LAST_DOM, 1);
      set_range(e->month, FIRST_MONTH, LAST_MONTH,
          FIRST_MONTH, LAST_MONTH, 1);
      set_range(e->dow, FIRST_DOW, LAST_DOW,
            FIRST_DOW, LAST_DOW, 1);
    } else if (!strcmp("hourly", cmd)) {
      set_element(e->minute, FIRST_MINUTE, LAST_MINUTE,
            FIRST_MINUTE);
      set_range(e->hour, FIRST_HOUR, LAST_HOUR,
          FIRST_HOUR, LAST_HOUR, 1);
      set_range(e->dom, FIRST_DOM, LAST_DOM,
          FIRST_DOM, LAST_DOM, 1);
      set_range(e->month, FIRST_MONTH, LAST_MONTH,
          FIRST_MONTH, LAST_MONTH, 1);
      set_range(e->dow, FIRST_DOW, LAST_DOW,
          FIRST_DOW, LAST_DOW, 1);
      e->flags |= HR_STAR;
    } else {
      goto eof;
    }
    /* Advance past whitespace between shortcut and
     * username/command.
     */
    Skip_Blanks(ch, file);
    if (ch == EOF || ch == '\n') {
      goto eof;
    }
  } else {
    Debug(DPARS, ("load_entry()...about to parse numerics\n"))

    if (ch == '*')
      e->flags |= MIN_STAR;
    ch = get_list(e->minute, FIRST_MINUTE, LAST_MINUTE,
            PPC_NULL, ch, file);
    if (ch == EOF) {
      goto eof;
    }

    /* hours
     */

    if (ch == '*')
      e->flags |= HR_STAR;
    ch = get_list(e->hour, FIRST_HOUR, LAST_HOUR,
            PPC_NULL, ch, file);
    if (ch == EOF) {
      goto eof;
    }

    /* DOM (days of month)
     */

    if (ch == '$') {
      ch = get_char(file);
      if (!Is_Blank(ch)) {
        goto eof;
      }
      Skip_Blanks(ch, file);
      e->flags |= DOM_LAST;
    } else {
      if (ch == '*')
        e->flags |= DOM_STAR;
      ch = get_list(e->dom, FIRST_DOM, LAST_DOM,
              PPC_NULL, ch, file);
    }
    if (ch == EOF) {
      goto eof;
    }

    /* month
     */

    ch = get_list(e->month, FIRST_MONTH, LAST_MONTH,
            MonthNames, ch, file);
    if (ch == EOF) {
      goto eof;
    }

    /* DOW (days of week)
     */

    if (ch == '*')
      e->flags |= DOW_STAR;
    ch = get_list(e->dow, FIRST_DOW, LAST_DOW,
            DowNames, ch, file);
    if (ch == EOF) {
      goto eof;
    }
  }

  /* make sundays equivalent */
  if (bit_test(e->dow, 0) || bit_test(e->dow, 7)) {
    bit_set(e->dow, 0);
    bit_set(e->dow, 7);
  }

  /* check for permature EOL and catch a common typo */
  if (ch == '\n' || ch == '*') {
    goto eof;
  }

  /* ch is the first character of a username */
  unget_char(ch, file);

  struct passwd *pw = NULL;
  char  *username = cmd;  /* temp buffer */

    Debug(DPARS, ("load_entry()...about to parse username\n"))
    ch = get_string(username, MAX_COMMAND, file, " \t\n");

    Debug(DPARS, ("load_entry()...got %s\n",username))
    if (ch == EOF || ch == '\n' || ch == '*') {
      goto eof;
    }

    /* Need to have consumed blanks before checking for options
     * below. */
    Skip_Blanks(ch, file)
    unget_char(ch, file);

    pw = getpwnam(username);
    if (pw == NULL) {
      log_it("USER", getpid(), username, "not found");
      goto eof;
    }
  log_it("USER", getpid(), username, "found");

  /* Allocate the name buffer */
  e->name = strdup(pw->pw_name);
  e->home = strdup(pw->pw_dir);
  e->shell = strdup(pw->pw_shell);

  e->uid=pw->pw_uid;
  e->gid=pw->pw_gid;

  /* Everything up to the next \n or EOF is part of the command...
   * too bad we don't know in advance how long it will be, since we
   * need to malloc a string for it... so, we limit it to MAX_COMMAND.
   */
  ch = get_string(cmd, MAX_COMMAND, file, "\n");

  /* a file without a \n before the EOF is rude, so we'll complain...
   */
  if (ch == EOF) {
    goto eof;
  }

  /* got the command in the 'cmd' string; save it in *e.
   */
  if ((e->cmd = strdup(cmd)) == NULL) {
    goto eof;
  }

  Debug(DPARS, ("load_entry()...returning successfully\n"))

  /* success, fini, return pointer to the entry we just created...
   */
  return (e);

 eof:
  free_entry(e);
  while (ch != '\n' && !feof(file))
    ch = get_char(file);
  return (NULL);
}

static int
get_list(bitstr_t *bits, int low, int high, const char *names[],
   int ch, FILE *file)
{
  int done;

  /* we know that we point to a non-blank character here;
   * must do a Skip_Blanks before we exit, so that the
   * next call (or the code that picks up the cmd) can
   * assume the same thing.
   */

  Debug(DPARS|DEXT, ("get_list()...entered\n"))

  /* list = range {"," range}
   */

  /* clear the bit string, since the default is 'off'.
   */
  bit_nclear(bits, 0, (high-low));

  /* process all ranges
   */
  done = FALSE;
  while (!done) {
    if (EOF == (ch = get_range(bits, low, high, names, ch, file)))
      return (EOF);
    if (ch == ',')
      ch = get_char(file);
    else
      done = TRUE;
  }

  /* exiting.  skip to some blanks, then skip over the blanks.
   */
  Skip_Nonblanks(ch, file)
  Skip_Blanks(ch, file)

  Debug(DPARS|DEXT, ("get_list()...exiting w/ %02x\n", ch))

  return (ch);
}


static int
get_range(bitstr_t *bits, int low, int high, const char *names[],
    int ch, FILE *file)
{
  /* range = number | number "-" number [ "/" number ]
   */

  int num1, num2, num3;

  Debug(DPARS|DEXT, ("get_range()...entering, exit won't show\n"))

  if (ch == '*') {
    /* '*' means "first-last" but can still be modified by /step
     */
    num1 = low;
    num2 = high;
    ch = get_char(file);
    if (ch == EOF)
      return (EOF);
  } else {
    ch = get_number(&num1, low, names, ch, file, ",- \t\n");
    if (ch == EOF)
      return (EOF);

    if (ch != '-') {
      /* not a range, it's a single number.
       */
      if (EOF == set_element(bits, low, high, num1)) {
        unget_char(ch, file);
        return (EOF);
      }
      return (ch);
    } else {
      /* eat the dash
       */
      ch = get_char(file);
      if (ch == EOF)
        return (EOF);

      /* get the number following the dash
       */
      ch = get_number(&num2, low, names, ch, file, "/, \t\n");
      if (ch == EOF || num1 > num2)
        return (EOF);
    }
  }

  /* check for step size
   */
  if (ch == '/') {
    /* eat the slash
     */
    ch = get_char(file);
    if (ch == EOF)
      return (EOF);

    /* get the step size -- note: we don't pass the
     * names here, because the number is not an
     * element id, it's a step size.  'low' is
     * sent as a 0 since there is no offset either.
     */
    ch = get_number(&num3, 0, PPC_NULL, ch, file, ", \t\n");
    if (ch == EOF || num3 == 0)
      return (EOF);
  } else {
    /* no step.  default==1.
     */
    num3 = 1;
  }

  /* range. set all elements from num1 to num2, stepping
   * by num3.  (the step is a downward-compatible extension
   * proposed conceptually by bob@acornrc, syntactically
   * designed then implemented by paul vixie).
   */
  if (EOF == set_range(bits, low, high, num1, num2, num3)) {
    unget_char(ch, file);
    return (EOF);
  }

  return (ch);
}

static int
get_number(int *numptr, int low, const char *names[], int ch, FILE *file,
    const char *terms) {
  char temp[MAX_TEMPSTR], *pc;
  int len, i;

  pc = temp;
  len = 0;

  /* first look for a number */
  while (isdigit((unsigned char)ch)) {
    if (++len >= MAX_TEMPSTR)
      goto bad;
    *pc++ = ch;
    ch = get_char(file);
  }
  *pc = '\0';
  if (len != 0) {
    /* got a number, check for valid terminator */
    if (!strchr(terms, ch))
      goto bad;
    i = atoi(temp);
    if (i < 0)
      goto bad;
    *numptr = i;
    return (ch);
  }

  /* no numbers, look for a string if we have any */
  if (names) {
    while (isalpha((unsigned char)ch)) {
      if (++len >= MAX_TEMPSTR)
        goto bad;
      *pc++ = ch;
      ch = get_char(file);
    }
    *pc = '\0';
    if (len != 0 && strchr(terms, ch)) {
      for (i = 0;  names[i] != NULL;  i++) {
        Debug(DPARS|DEXT,
          ("get_num, compare(%s,%s)\n", names[i],
          temp))
        if (!strcasecmp(names[i], temp)) {
          *numptr = i+low;
          return (ch);
        }
      }
    }
  }

bad:
  unget_char(ch, file);
  return (EOF);
}

static int
set_element(bitstr_t *bits, int low, int high, int number) {
  Debug(DPARS|DEXT, ("set_element(?,%d,%d,%d)\n", low, high, number))

  if (number < low || number > high)
    return (EOF);
  number -= low;

  bit_set(bits, number);
  return (OK);
}

static int
set_range(bitstr_t *bits, int low, int high, int start, int stop, int step) {
  Debug(DPARS|DEXT, ("set_range(?,%d,%d,%d,%d,%d)\n",
         low, high, start, stop, step))

  if (start < low || stop > high)
    return (EOF);
  start -= low;
  stop -= low;

  if (step <= 1 || step > stop) {
    bit_nset(bits, start, stop);
  } else {
    for (int i = start; i <= stop; i += step)
      bit_set(bits, i);
  }
  return (OK);
}

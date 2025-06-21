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
static char rcsid[] = "$Id: do_command.c,v 1.12 2021/02/07 00:20:00 vixie Exp $";
#endif

#include "cron.h"

static void		child_process(const entry *, const user *);
static int		safe_p(const char *, const char *);

void
do_command(const entry *e, const user *u) {
	Debug(DPROC, ("[%ld] do_command(%s, (%s,%ld,%ld))\n",
		      (long)getpid(), e->cmd, u->name,
		      (long)e->pwd->pw_uid, (long)e->pwd->pw_gid))

	/* fork to become asynchronous -- parent process is done immediately,
	 * and continues to run the normal cron code, which means return to
	 * tick().  the child and grandchild don't leave this function, alive.
	 *
	 * vfork() is unsuitable, since we have much to do, and the parent
	 * needs to be able to run off and fork other processes.
	 */
	switch (fork()) {
	case -1:
		log_it("CRON", getpid(), "error", "can't fork");
		break;
	case 0:
		/* child process */
		acquire_daemonlock(1);
		child_process(e, u);
		Debug(DPROC, ("[%ld] child process done, exiting\n",
			      (long)getpid()))
		_exit(OK_EXIT);
		break;
	default:
		/* parent process */
		break;
	}
	Debug(DPROC, ("[%ld] main process returning to work\n",(long)getpid()))
}

static void
child_process(const entry *e, const user *u) {
	int stdin_pipe[2], stdout_pipe[2];
	char *input_data, *usernm;
	int children = 0;

	Debug(DPROC, ("[%ld] child_process('%s')\n", (long)getpid(), e->cmd))

	/* discover some useful and important environment settings
	 */
	usernm = e->pwd->pw_name;

	/* our parent is watching for our death by catching SIGCHLD.  we
	 * do not care to watch for our childrens' deaths this way -- we
	 * use wait() explicitly.  so we have to reset the signal (which
	 * was inherited from the parent).
	 */
	(void) signal(SIGCHLD, SIG_DFL);

	/* create some pipes to talk to our future child
	 */
	pipe(stdin_pipe);	/* child's stdin */
	pipe(stdout_pipe);	/* child's stdout */
	
	/* since we are a forked process, we can modify the command string
	 * we were passed -- nobody else is going to use it again, right?
	 *
	 * if a % is present in the command, previous characters are the
	 * command, and subsequent characters are the additional input to
	 * the command.  An escaped % will have the escape character stripped
	 * from it.  Subsequent %'s will be transformed into newlines,
	 * but that happens later.
	 */
	/*local*/{
		int escaped = FALSE;
		int ch;
		char *p;

		for (input_data = p = e->cmd;
		     (ch = *input_data) != '\0';
		     input_data++, p++) {
			if (p != input_data)
				*p = ch;
			if (escaped) {
				if (ch == '%')
					*--p = ch;
				escaped = FALSE;
				continue;
			}
			if (ch == '\\') {
				escaped = TRUE;
				continue;
			}
			if (ch == '%') {
				*input_data++ = '\0';
				break;
			}
		}
		*p = '\0';
	}

	/* fork again, this time so we can exec the user's command.
	 */
	switch (vfork()) {
	case -1:
		log_it("CRON", getpid(), "error", "can't vfork");
		exit(ERROR_EXIT);
		/*NOTREACHED*/
	case 0:
		Debug(DPROC, ("[%ld] grandchild process vfork()'ed\n",
			      (long)getpid()))

		/* write a log message.  we've waited this long to do it
		 * because it was not until now that we knew the PID that
		 * the actual user command shell was going to get and the
		 * PID is part of the log message.
		 */
		if ((e->flags & DONT_LOG) == 0) {
			char *x = mkprints((u_char *)e->cmd, strlen(e->cmd));

			log_it(usernm, getpid(), "CMD", x);
			free(x);
		}

		/* that's the last thing we'll log.  close the log files.
		 */
		log_close();

		/* get new pgrp, void tty, etc.
		 */
		(void) setsid();

		/* close the pipe ends that we won't use.  this doesn't affect
		 * the parent, who has to read and write them; it keeps the
		 * kernel from recording us as a potential client TWICE --
		 * which would keep it from sending SIGPIPE in otherwise
		 * appropriate circumstances.
		 */
		close(stdin_pipe[WRITE_PIPE]);
		close(stdout_pipe[READ_PIPE]);

		/* grandchild process.  make std{in,out} be the ends of
		 * pipes opened by our daddy; make stderr go to stdout.
		 */
		if (stdin_pipe[READ_PIPE] != STDIN) {
			dup2(stdin_pipe[READ_PIPE], STDIN);
			close(stdin_pipe[READ_PIPE]);
		}
		if (stdout_pipe[WRITE_PIPE] != STDOUT) {
			dup2(stdout_pipe[WRITE_PIPE], STDOUT);
			close(stdout_pipe[WRITE_PIPE]);
		}
		dup2(STDOUT, STDERR);

		setgid(e->pwd->pw_gid);
		initgroups(usernm, e->pwd->pw_gid);
#if (defined(BSD)) && (BSD >= 199103)
		setlogin(usernm);
#endif /* BSD */
		if (setuid(e->pwd->pw_uid) < 0) {
			perror("setuid");
			_exit(ERROR_EXIT);
		}
		/* we aren't root after this... */

		chdir(env_get("HOME", e->envp));

		/*
		 * Exec the command.
		 */
		{
			char	*shell = env_get("SHELL", e->envp);
			Debug(DPROC, ("cmd='%s' shell='%s'", e->cmd, shell));
			execle(shell, shell, "-c", e->cmd, (char *)0, e->envp);
			fprintf(stderr, "execl: couldn't exec `%s'\n", shell);
			perror("execl");
			_exit(ERROR_EXIT);
		}
		break;
	default:
		/* parent process */
		break;
	}

	children++;

	/* middle process, child of original cron, parent of process running
	 * the user's command.
	 */

	Debug(DPROC, ("[%ld] child continues, closing pipes\n",(long)getpid()))

	/* close the ends of the pipe that will only be referenced in the
	 * grandchild process...
	 */
	close(stdin_pipe[READ_PIPE]);
	close(stdout_pipe[WRITE_PIPE]);

	/* write, to the pipe connected to child's stdin, any input specified
	 * after a % in the crontab entry.  while we copy, convert any
	 * additional %'s to newlines.  when done, if some characters were
	 * written and the last one wasn't a newline, write a newline.
	 *
	 * Note that if the input data won't fit into one pipe buffer (2K
	 * or 4K on most BSD systems), and the child doesn't read its stdin,
	 * we would block here.  thus we must fork again.
	 */

	if (*input_data && fork() == 0) {
		FILE *out = fdopen(stdin_pipe[WRITE_PIPE], "w");
		int need_newline = FALSE;
		int escaped = FALSE;
		int ch;

		Debug(DPROC, ("[%ld] child2 sending data to grandchild\n",
			      (long)getpid()))

		/* close the pipe we don't use, since we inherited it and
		 * are part of its reference count now.
		 */
		close(stdout_pipe[READ_PIPE]);

		/* translation:
		 *	\% -> %
		 *	%  -> \n
		 *	\x -> \x	for all x != %
		 */
		while ((ch = *input_data++) != '\0') {
			if (escaped) {
				if (ch != '%')
					putc('\\', out);
			} else {
				if (ch == '%')
					ch = '\n';
			}

			if (!(escaped = (ch == '\\'))) {
				putc(ch, out);
				need_newline = (ch != '\n');
			}
		}
		if (escaped)
			putc('\\', out);
		if (need_newline)
			putc('\n', out);

		/* close the pipe, causing an EOF condition.  fclose causes
		 * stdin_pipe[WRITE_PIPE] to be closed, too.
		 */
		fclose(out);

		Debug(DPROC, ("[%ld] child2 done sending to grandchild\n",
			      (long)getpid()))
		exit(0);
	}

	/* close the pipe to the grandkiddie's stdin, since its wicked uncle
	 * ernie back there has it open and will close it when he's done.
	 */
	close(stdin_pipe[WRITE_PIPE]);

	children++;

	/* read output from the grandchild.  its stderr has been redirected to
	 * it's stdout, which has been redirected to our pipe.  if there is any
	 * output, we'll be mailing it to the user whose crontab this is...
	 * when the grandchild exits, we'll get EOF.
	 */

	Debug(DPROC, ("[%ld] child reading output from grandchild\n",
		      (long)getpid()))

	/*local*/{
		FILE	*in = fdopen(stdout_pipe[READ_PIPE], "r");
		int	ch = getc(in);

		if (ch != EOF) {
			int	bytes = 1;
			int	status = 0;

			Debug(DPROC|DEXT,
			      ("[%ld] got data (%x:%c) from grandchild\n",
			       (long)getpid(), ch, ch))

			/* we have to read the input pipe no matter whether
			 * we mail or not, but obviously we only write to
			 * mail pipe if we ARE mailing.
			 */

			while (EOF != (ch = getc(in))) {
				bytes++;
			}

		} /*if data from grandchild*/

		Debug(DPROC, ("[%ld] got EOF from grandchild\n",
			      (long)getpid()))

		fclose(in);	/* also closes stdout_pipe[READ_PIPE] */
	}

	/* wait for children to die.
	 */
	for (; children > 0; children--) {
		WAIT_T waiter;
		PID_T pid;

		Debug(DPROC, ("[%ld] waiting for grandchild #%d to finish\n",
			      (long)getpid(), children))
		while ((pid = wait(&waiter)) < OK && errno == EINTR)
			;
		if (pid < OK) {
			Debug(DPROC,
			      ("[%ld] no more grandchildren--mail written?\n",
			       (long)getpid()))
			break;
		}
		Debug(DPROC, ("[%ld] grandchild #%ld finished, status=%04x",
			      (long)getpid(), (long)pid, WEXITSTATUS(waiter)))
		if (WIFSIGNALED(waiter) && WCOREDUMP(waiter))
			Debug(DPROC, (", dumped core"))
		Debug(DPROC, ("\n"))
	}
}

static int
safe_p(const char *usernm, const char *s) {
	static const char safe_delim[] = "@!:%-.,";     /* conservative! */
	const char *t;
	int ch, first;

	for (t = s, first = 1; (ch = *t++) != '\0'; first = 0) {
		if (isascii(ch) && isprint(ch) &&
		    (isalnum(ch) || (!first && strchr(safe_delim, ch))))
			continue;
		log_it(usernm, getpid(), "UNSAFE", s);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Copyright (c) 2000,2002 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND TODD C. MILLER DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL TODD C. MILLER BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copyright (c) 2021 by Paul Vixie ("VIXIE")
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
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
static char rcsid[] = "$Id: pw_dup.c,v 1.2 2004/01/23 18:56:43 vixie Exp $";
#endif

#include <sys/param.h>

#if !defined(OpenBSD) || OpenBSD < 200105

#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct passwd *
pw_dup(const struct passwd *pw) {
	char		*cp;
	size_t		 nsize, psize, csize, gsize, dsize, ssize, total;
	struct passwd	*newpw;

	/* Allocate in one big chunk for easy freeing */
	total = sizeof(struct passwd);
	if (pw->pw_name) {
		nsize = strlen(pw->pw_name) + 1;
		total += nsize;
	}
	if (pw->pw_passwd) {
		psize = strlen(pw->pw_passwd) + 1;
		total += psize;
	}
	if (pw->pw_gecos) {
		gsize = strlen(pw->pw_gecos) + 1;
		total += gsize;
	}
	if (pw->pw_dir) {
		dsize = strlen(pw->pw_dir) + 1;
		total += dsize;
	}
	if (pw->pw_shell) {
		ssize = strlen(pw->pw_shell) + 1;
		total += ssize;
	}
	if ((cp = malloc(total)) == NULL)
		return (NULL);
	newpw = (struct passwd *)cp;

	/*
	 * Copy in passwd contents and make strings relative to space
	 * at the end of the buffer.
	 */
	(void)memcpy(newpw, pw, sizeof(struct passwd));
	cp += sizeof(struct passwd);
	if (pw->pw_name) {
		(void)memcpy(cp, pw->pw_name, nsize);
		newpw->pw_name = cp;
		cp += nsize;
	}
	if (pw->pw_passwd) {
		(void)memcpy(cp, pw->pw_passwd, psize);
		newpw->pw_passwd = cp;
		cp += psize;
	}
	if (pw->pw_gecos) {
		(void)memcpy(cp, pw->pw_gecos, gsize);
		newpw->pw_gecos = cp;
		cp += gsize;
	}
	if (pw->pw_dir) {
		(void)memcpy(cp, pw->pw_dir, dsize);
		newpw->pw_dir = cp;
		cp += dsize;
	}
	if (pw->pw_shell) {
		(void)memcpy(cp, pw->pw_shell, ssize);
		newpw->pw_shell = cp;
		cp += ssize;
	}

	return (newpw);
}

#endif /* !OpenBSD || OpenBSD < 200105 */

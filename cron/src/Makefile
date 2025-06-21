##
## Copyright (c) 1988,1990,1993,1994,2021 by Paul Vixie ("VIXIE")
## Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
## Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
##
## Permission to use, copy, modify, and distribute this software for any
## purpose with or without fee is hereby granted, provided that the above
## copyright notice and this permission notice appear in all copies.
##
## THE SOFTWARE IS PROVIDED "AS IS" AND VIXIE DISCLAIMS ALL WARRANTIES
## WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
## MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL VIXIE BE LIABLE FOR
## ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
## WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
## ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
## OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
##

# Makefile for VIXIE cron
#
# $Id: Makefile,v 1.9 2004/01/23 18:56:42 vixie Exp $
#
# vix 03mar88 [moved to RCS, rest of log is in there]
# vix 30mar87 [goodbye, time.c; hello, getopt]
# vix 12feb87 [cleanup for distribution]
# vix 30dec86 [written]

# NOTES:
#	'make' can be done by anyone
#	'make install' must be done by root
#
#	this package needs getopt(3), bitstring(3), and BSD install(8).
#
#	the configurable stuff in this makefile consists of compilation
#	options (use -O, cron runs forever) and destination directories.
#	SHELL is for the 'augumented make' systems where 'make' imports
#	SHELL from the environment and then uses it to run its commands.
#	if your environment SHELL variable is /bin/csh, make goes real
#	slow and sometimes does the wrong thing.  
#
#	this package needs the 'bitstring macros' library, which is
#	available from me or from the comp.sources.unix archive.  if you
#	put 'bitstring.h' in a non-standard place (i.e., not intuited by
#	cc(1)), you will have to define INCLUDE to set the include
#	directory for cc.  INCLUDE should be `-Isomethingorother'.
#
#	there's more configuration info in config.h; edit that first!

#################################### begin configurable stuff
#<<need bitstring.h>>
INCLUDE		=	-I.
#INCLUDE	=
#CDEBUG		=	-O
CDEBUG		=	-g
#<<assume gcc or clang>>
CWARN		=	-Wall -Wno-unused -Wno-comment
#<<manifest defines>>
DEFS		=
LDFLAGS		= -s
#################################### end configurable stuff

SHELL		=	/bin/sh
CFLAGS		=	$(CDEBUG) $(CWARN) $(INCLUDE) $(DEFS)

HEADERS	=	bitstring.h cron.h pathnames.h externs.h macros.h structs.h funcs.h globals.h
SOURCES	=	cron.c database.c do_command.c entry.c env.c job.c log.c user.c misc.c popen.c pw_dup.c
OBJECTS	=	cron.o database.o do_command.o entry.o env.o job.o log.o user.o misc.o popen.o pw_dup.o


all: cron

cron: $(OBJECTS)
			$(CC) $(LDFLAGS) -o cron $(OBJECTS)

$(OBJECTS): $(HEADERS) Makefile

clean:
			rm -f *~
			rm -f *.o
			rm -f cron


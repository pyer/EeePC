# Makefile for cron
#
# NOTES:
#
#	the configurable stuff in this makefile consists of compilation
#	options (use -O, cron runs forever) and destination directories.
#	SHELL is for the 'augumented make' systems where 'make' imports
#	SHELL from the environment and then uses it to run its commands.
#	if your environment SHELL variable is /bin/csh, make goes real
#	slow and sometimes does the wrong thing.  
#
#	there's more configuration info in config.h; edit that first!

#################################### begin configurable stuff
#CDEBUG	= -O
CDEBUG	= -g
#<<assume gcc or clang>>
CWARN	= -Wall -Wno-unused -Wno-comment
LDFLAGS	= -s
#################################### end configurable stuff

SHELL	= /bin/sh
CFLAGS	= $(CDEBUG) $(CWARN)

HEADERS	= bitstring.h cron.h macros.h structs.h funcs.h globals.h
SOURCES	= cron.c database.c do_command.c entry.c job.c log.c misc.c
OBJECTS	= cron.o database.o do_command.o entry.o job.o log.o misc.o


all: cron

cron: $(OBJECTS)
	$(CC) $(LDFLAGS) -o cron $(OBJECTS)

$(OBJECTS): $(HEADERS) Makefile

clean:
	rm -f *~
	rm -f *.o
	rm -f cron


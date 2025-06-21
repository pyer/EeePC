
/*
 * log.c
 *
 */

#include "externs.h"

#define LOG_FILE "/var/log/cron.log"

/* system calls return this on success */
#define OK		0
/*   or this on error */
#define ERR		(-1)

static int LogFD = ERR;

void
log_it(const char *username, PID_T xpid, const char *event, const char *detail) {
#if defined(LOG_FILE)
	char *msg;
	TIME_T now = time((TIME_T) 0);
	struct tm *t = localtime(&now);
#endif /*LOG_FILE*/

#if defined(LOG_FILE)
	/* we assume that 120 bytes will hold the date, time, &punctuation.
	 */
	msg = malloc(strlen(username)
		     + strlen(event)
		     + strlen(detail)
		     + 120);
	if (msg == NULL)
		return;

	if (LogFD < OK) {
		LogFD = open(LOG_FILE, O_WRONLY|O_APPEND|O_CREAT, 0644);
		if (LogFD < OK) {
			perror(LOG_FILE);
		} else {
			(void) fcntl(LogFD, F_SETFD, 1);
		}
	}

	/* we have to sprintf() it because fprintf() doesn't always write
	 * everything out in one chunk and this has to be atomically appended
	 * to the log file.
	 */
	sprintf(msg, "%s (%02d/%02d-%02d:%02d:%02d-%d) %s (%s)\n",
		username,
		t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, xpid,
		event, detail);

	/* we have to run strlen() because sprintf() returns (char*) on old BSD
	 */
	if (LogFD < OK || write(LogFD, msg, strlen(msg)) < OK) {
			perror(LOG_FILE);
	}

	free(msg);
#endif /*LOG_FILE*/
}

void log_close(void) {
	if (LogFD != ERR) {
		close(LogFD);
		LogFD = ERR;
	}
}

void log_debug(int mask, char *msg) {
  int fd = open(LOG_FILE, O_WRONLY|O_APPEND|O_CREAT, 0644);
  if (fd >= 0) {
	  write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
		close(fd);
  }
}

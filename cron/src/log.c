
/*
 * log.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#define LOG_FILE "/var/log/cron.log"

void log_it(const char *username, pid_t xpid, const char *event, const char *detail) {
  int len = 0;
  time_t now = time(0);
  struct tm *t = localtime(&now);

  /* we assume that 120 bytes will hold the date, time, &punctuation.
   */
  char *msg = malloc(strlen(username)
         + strlen(event)
         + strlen(detail)
         + 120);
  if (msg == NULL)
    return;

  int fd = open(LOG_FILE, O_WRONLY|O_APPEND|O_CREAT, 0644);
  if (fd < 0) {
    perror(LOG_FILE);
  } else {
    //(void) fcntl(LogFD, F_SETFD, 1);

    /* we have to sprintf() it because fprintf() doesn't always write
     * everything out in one chunk and this has to be atomically appended
     * to the log file.
     */
    len = sprintf(msg, "%04d-%02d-%02d %02d:%02d:%02d %-6s [%d] %s: %s\n",
                t->tm_year + 1900,
                t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,
                username, xpid, event, detail);
     /* What to do in case of error ? */
     write(fd, msg, len);
     close(fd);
  }

  free(msg);
}



#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "str.h"
#include "uw_tmp.h"

#define FATAL "utmpset: fatal: "
#define WARNING "utmpset: warning: "

const char *progname;

int open_uw_tmp(const char *filename, int flags, int mode) {
  int fd = open(filename, flags, mode);
  if (fd < 0) {
    log_error("utmpset", "unable to open ", filename);
    _exit(111);
  }
  if (flock(fd,LOCK_EX) == -1) {
    log_error("utmpset", "unable to lock ", filename);
    _exit(111);
  }
  return(fd);
}

int utmp_logout(const char *line) {
  int fd;
  uw_tmp ut;
  int ok = -1;

/* open R/W */
  fd = open_uw_tmp(UW_TMP_UFILE, O_RDWR, 0);
  while (read(fd, &ut, sizeof(uw_tmp)) == sizeof(uw_tmp)) {
    if (!ut.ut_name[0] || (!str_equal(ut.ut_line, line)))
      continue;
    memset(ut.ut_name, 0, sizeof ut.ut_name);
    memset(ut.ut_host, 0, sizeof ut.ut_host);
    ut.ut_time = time(NULL);
    if (ut.ut_time == -1)
      break;
#ifdef DEAD_PROCESS
    ut.ut_type =DEAD_PROCESS;
#endif
    if (lseek(fd, -(off_t)sizeof(uw_tmp), SEEK_CUR) == -1)
      break;
    if (write(fd, &ut, sizeof(uw_tmp)) != sizeof(uw_tmp))
      break;
    ok = 0;
    break;
  }
  close(fd);
  return(ok);
}

int wtmp_logout(const char *line) {
  int fd;
  int len;
  struct stat st;
  uw_tmp ut;

/* append or create */
  fd = open_uw_tmp(UW_TMP_WFILE, O_WRONLY | O_NDELAY | O_APPEND | O_CREAT, 0600);
  if (fstat(fd, &st) == -1) {
    close(fd);
    return(-1);
  }
  memset(&ut, 0, sizeof(uw_tmp));
  if ((len =str_len(line)) > sizeof ut.ut_line)
    len =sizeof ut.ut_line - 2;
  memcpy(ut.ut_line, line, len);
  ut.ut_time = time(NULL);
  if (ut.ut_time == -1) {
    close(fd);
    return(-1);
  }
#ifdef DEAD_PROCESS
  ut.ut_type =DEAD_PROCESS;
#endif
  if (write(fd, &ut, sizeof(uw_tmp)) != sizeof(uw_tmp)) {
    ftruncate(fd, st.st_size);
    close(fd);
    return(-1);
  }
  close(fd);
  return(0);
}

int main (int argc, const char * const *argv, const char * const *envp) {
  argv++;
  if (argc != 2) {
    log_error("utmpset", "Usage: utmpset line", 0);
    _exit(111);
  }

  if (utmp_logout(*argv) == -1) {
//    log_error(UW_TMP_UFILE, "unable to logout line ", *argv);
    _exit(111);
  }
  if (wtmp_logout(*argv) == -1) {
//    log_error(UW_TMP_WFILE, "unable to logout line ", *argv);
    _exit(111);
  }
  _exit(0);
}

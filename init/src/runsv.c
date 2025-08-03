
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "sig.h"
#include "str.h"
#include "wait.h"

char *progname;
char * const *environ;

int selfpipe[2];

/* state */
#define S_DOWN 0
#define S_RUN 1
#define S_FINISH 2
/* ctrl */
#define C_NOOP 0
#define C_TERM 1
#define C_PAUSE 2
/* want */
#define W_UP 0
#define W_DOWN 1
#define W_EXIT 2

struct svdir {
  int pid;
  int state;
  int ctrl;
  int want;
  time_t start;
  int wstat;
  int fdcontrol;
};
struct svdir svd[2];

int sigterm =0;
int pidchanged =1;
char *dir;


void fatal(char *m1, char *m2) {
//  strerr_die6sys(111, "runsv ", dir, ": fatal: ", m1, m2, ": ");
  log_error(dir, m1, m2);
  _exit(111);
}

void warn(char *m1, char *m2) {
//  strerr_warn6("runsv ", dir, ": warning: ", m1, m2, ": ", &strerr_sys);
  log_warn(dir, m1, m2);
}

#define FMT_ULONG 40 /* enough space to hold 2^128 - 1 in decimal, plus \0 */

unsigned int fmt_ulong(register char *s,register unsigned long u)
{
  register unsigned int len = 1;
  register unsigned long q = u;
  while (q > 9) {
    ++len; q /= 10;
  }
  if (s) {
    s += len;
    do {
      *--s = '0' + (u % 10);
      u /= 10;
    } while(u); /* handles u == 0 */
  }
  return len;
}

void stopservice(struct svdir *);

void s_child() { write(selfpipe[1], "", 1); }
void s_term() {
  sigterm =1;
  write(selfpipe[1], "", 1); /* XXX */
}

void update_status(struct svdir *s) {
  int fd;
  char spid[FMT_ULONG];
  char *fpid ="supervise/pid";
  char *fpidnew ="supervise/pid.new";
  char *fstatus ="supervise/status";
  char *fstatusnew ="supervise/status.new";

  /* pid */
  if (pidchanged) {
    if ((fd = open(fpidnew, O_WRONLY | O_NONBLOCK | O_TRUNC | O_CREAT, 0644)) == -1) {
      warn("unable to open ", fpidnew);
      return;
    }
    spid[fmt_ulong(spid, (unsigned long)s->pid)] = 0;
    if (s->pid) {
      write(fd,spid,str_len(spid));
      write(fd,"\n",1);
    }
    close(fd);
    if (rename(fpidnew, fpid) == -1) {
      warn("unable to rename pid.new to ", fpid);
      return;
    }
    pidchanged =0;
  }

  /* status */
  if ((fd = open(fstatusnew, O_WRONLY | O_NONBLOCK | O_TRUNC | O_CREAT, 0644)) == -1) {
    warn("unable to open ", fstatusnew);
    return;
  }
  switch (s->state) {
  case S_DOWN:
    write(fd,"down",4);
    break;
  case S_RUN:
    write(fd,"run",3);
    break;
  case S_FINISH:
    write(fd,"finish",6);
    break;
  }
  if (s->ctrl & C_PAUSE)
    write(fd," paused",7);
  if (s->ctrl & C_TERM)
    write(fd,", got TERM",10);
  if (s->state != S_DOWN)
    switch(s->want) {
    case W_DOWN:
      write(fd,", want down",11);
      break;
    case W_EXIT:
      write(fd,", want exit",11);
      break;
    }
  write(fd,"\n",1);
  close(fd);
  if (rename(fstatusnew, fstatus) == -1)
    warn("unable to rename status.new to ", fstatus);
}

unsigned int custom(struct svdir *s, char c) {
  int pid;
  int w;
  struct stat st;
  char *prog[2];
  char a[10] = "control/?";

  a[8] = c;
  if (stat(a, &st) == 0) {
    if (st.st_mode & S_IXUSR) {
      if ((pid =fork()) == -1) {
        warn("unable to fork for ", a);
        return(0);
      }
      if (! pid) {
        prog[0] =a;
        prog[1] =0;
        execve(a, prog, environ);
        fatal("unable to run control/? for ", a);
      }
      while (wait_pid(&w, pid) == -1) {
        if (errno == EINTR)
          continue;
        warn("unable to wait for child ", a);
        return(0);
      }
      return(! wait_exitcode(w));
    }
  }
  else {
    if (errno == ENOENT)
      return(0);
    warn("unable to stat ", a);
  }
  return(0);
}

void stopservice(struct svdir *s) {
  if (s->pid && ! custom(s, 't')) {
    kill(s->pid, SIGTERM);
    s->ctrl |=C_TERM;
    update_status(s);
  }
  if (s->want == W_DOWN) {
    kill(s->pid, SIGCONT);
    custom(s, 'd'); return;
  }
  if (s->want == W_EXIT) {
    kill(s->pid, SIGCONT);
    custom(s, 'x');
  }
}

void startservice(struct svdir *s) {
  int p;
  char *run[4];
  char code[FMT_ULONG];
  char stat[FMT_ULONG];

  if (s->state == S_FINISH) {
    run[0] ="./finish";
    code[fmt_ulong(code, wait_exitcode(s->wstat))] =0;
    run[1] =wait_crashed(s->wstat) ? "-1" : code;
    stat[fmt_ulong(stat, s->wstat & 0xff)] =0;
    run[2] =stat;
    run[3] =0;
  }
  else {
    run[0] ="./run";
    custom(s, 'u');
    run[1] =0;
  }

  if (s->pid != 0) stopservice(s); /* should never happen */
  while ((p =fork()) == -1) {
    warn("unable to fork, sleeping", 0);
    sleep(5);
  }
  if (p == 0) {
    /* child */
    sig_default(SIGCHLD);
    sig_unblock(SIGCHLD);
    sig_default(SIGTERM);
    sig_unblock(SIGTERM);
    execve(*run, run, environ);
    fatal("unable to execute ", *run);
  }
  if (s->state != S_FINISH) {
    s->start = time(NULL);
    s->state = S_RUN;
  }
  s->pid =p;
  pidchanged =1;
  s->ctrl =C_NOOP;
  update_status(s);
}
int ctrl(struct svdir *s, char c) {
  switch(c) {
  case 'd': /* down */
    s->want =W_DOWN;
    update_status(s);
    if (s->state == S_RUN) stopservice(s);
    break;
  case 'u': /* up */
    s->want =W_UP;
    update_status(s);
    if (s->state == S_DOWN) startservice(s);
    break;
  case 'x': /* exit */
    s->want =W_EXIT;
    update_status(s);
    if (s->state == S_RUN) stopservice(s);
    break;
  case 't': /* sig term */
    if (s->state == S_RUN) stopservice(s);
    break;
  case 'k': /* sig kill */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGKILL);
    s->state =S_DOWN;
    break;
  case 'p': /* sig pause */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGSTOP);
    s->ctrl |=C_PAUSE;
    update_status(s);
    break;
  case 'c': /* sig cont */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGCONT);
    if (s->ctrl & C_PAUSE) s->ctrl &=~C_PAUSE;
    update_status(s);
    break;
  case 'o': /* once */
    s->want =W_DOWN;
    update_status(s);
    if (s->state == S_DOWN) startservice(s);
    break;
  case 'a': /* sig alarm */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGALRM);
    break;
  case 'h': /* sig hup */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGHUP);
    break;
  case 'i': /* sig int */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGINT);
    break;
  case 'q': /* sig quit */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGQUIT);
    break;
  case '1': /* sig usr1 */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGUSR1);
    break;
  case '2': /* sig usr2 */
    if ((s->state == S_RUN) && ! custom(s, c)) kill(s->pid, SIGUSR2);
    break;
  }
  return(1);
}

/*
 * main entry
 */
int main(int argc, char *argv[], char * const *envp) {
  struct stat s;
  int fd;
  int r;
  char buf[256];

  progname = argv[0];
  environ  = envp;
  if (! argv[1] || argv[2]) {
    fatal("Usage: runsv <service>", 0);
    _exit(1);
  }
  dir = argv[1];

  if (pipe(selfpipe) == -1)
    fatal("unable to create selfpipe", 0);
  fcntl(selfpipe[0], F_SETFD, 1);
  fcntl(selfpipe[1], F_SETFD, 1);
  fcntl(selfpipe[0], F_SETFL,fcntl(selfpipe[0], F_GETFL, 0) | O_NONBLOCK);
  fcntl(selfpipe[1], F_SETFL,fcntl(selfpipe[1], F_GETFL, 0) | O_NONBLOCK);
  
  sig_block(SIGCHLD);
  sig_catch(SIGCHLD, s_child);
  sig_block(SIGTERM);
  sig_catch(SIGTERM, s_term);

  if (chdir(dir) == -1)
    fatal("unable to change to directory ", dir);
  svd[0].pid =0;
  svd[0].state =S_DOWN;
  svd[0].ctrl =C_NOOP;
  svd[0].want =W_UP;
  svd[1].pid =0;
  if (stat("down", &s) != -1) svd[0].want =W_DOWN;

  if (mkdir("supervise", 0700) == -1) {
    if ((r =readlink("supervise", buf, 256)) != -1) {
      if (r == 256) {
        errno = 75;
        fatal("unable to readlink ./supervise", 0);
      }
      buf[r] =0;
      mkdir(buf, 0700);
    }
    else {
      if ((errno != ENOENT) && (errno != EINVAL))
        fatal("unable to readlink ./supervise", 0);
    }
  }

  mkfifo("supervise/control", 0600);
  if (stat("supervise/control", &s) == -1)
    fatal("unable to stat supervise/control", 0);
  if (!S_ISFIFO(s.st_mode))
    fatal("supervise/control exists but is not a fifo", 0);
  if ((svd[0].fdcontrol = open("supervise/control", O_RDONLY | O_NONBLOCK)) == -1)
    fatal("unable to open supervise/control", 0);
  fcntl(svd[0].fdcontrol, F_SETFD, 1);
  update_status(&svd[0]);

  for (;;) {
    struct pollfd x[3];
    char ch;

    if (! svd[0].pid)
      if ((svd[0].want == W_UP) || (svd[0].state == S_FINISH))
        startservice(&svd[0]);

    x[0].fd = selfpipe[0];
    x[0].events = POLLIN;
    x[0].revents = 0;
    x[1].fd = svd[0].fdcontrol;
    x[1].events = POLLIN;
    x[1].revents = 0;

    sig_unblock(SIGTERM);
    sig_unblock(SIGCHLD);
    poll(x, 2, 1000);
    sig_block(SIGTERM);
    sig_block(SIGCHLD);

    while (read(selfpipe[0], &ch, 1) == 1)
      ;
    for (;;) {
      int child;
      int wstat;
      
      child =wait_nohang(&wstat);
      if (!child)
        break;
      if ((child == -1) && (errno != EINTR))
        break;
      if (child == svd[0].pid) {
        svd[0].pid =0;
        pidchanged =1;
        svd[0].wstat =wstat;
        svd[0].ctrl &=~C_TERM;
        if (svd[0].state != S_FINISH)
          if ((fd = open("finish", O_RDONLY | O_NONBLOCK)) != -1) {
            close(fd);
            svd[0].state =S_FINISH;
            update_status(&svd[0]);
            continue;
          }
        svd[0].state =S_DOWN;
        update_status(&svd[0]);
      }
    }
    if (read(svd[0].fdcontrol, &ch, 1) == 1) ctrl(&svd[0], ch);

    if (sigterm) { ctrl(&svd[0], 'x'); sigterm =0; }

    if ((svd[0].want == W_EXIT) && (svd[0].state == S_DOWN)) {
      if (svd[1].pid == 0) _exit(0);
      if (svd[1].want != W_EXIT) {
        svd[1].want =W_EXIT;
        /* stopservice(&svd[1]); */
        update_status(&svd[1]);
      }
    }
  }
  _exit(0);
}

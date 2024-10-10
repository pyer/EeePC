#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define LENGTH 300
#define LOOPS 10000000
#define BILLION  1000000000L

struct timespec start, stop;

void start_time() {
    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
    }
}

void stop_time() {
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
      perror( "clock gettime" );
    }
}

void elapsed_time(const char *prefix) {
    double accum;
    accum = ( stop.tv_sec - start.tv_sec )
             + (double)( stop.tv_nsec - start.tv_nsec )
               / (double)BILLION;
    printf( "%8s : %lf\n", prefix, accum );
}


unsigned int str_len(const char *s)
{
  register const char *t;

  t = s;
  for (;;) {
    if (!*t) return t - s; ++t;
    if (!*t) return t - s; ++t;
    if (!*t) return t - s; ++t;
    if (!*t) return t - s; ++t;
  }
}

/* write length */
size_t wlen(const char *s)
{
  register const char *t = s;
  while(*t)
    t++;
  return(t-s);
}

size_t slen(const char *s)
{
  register const char *t = s;
  while(*t++);
  return(t-s);
}

size_t xlen(const char *s)
{
  register const char *t = s;
  for (;;) {
    if (!*t++)
      return(t-s);
    if (!*t++)
      return(t-s);
    if (!*t++)
      return(t-s);
    if (!*t++)
      return(t-s);
  }
}


char buffer[LENGTH];

/*
 * main entry
 */
int main(int argc, char *argv[]) {
  int n = 0;

  for( int i=0; i<LENGTH; i++) {
    buffer[i]='x';
  }
  buffer[LENGTH-1]=0;

  start_time();
  n = LOOPS;
  while( n-- ) {
    str_len(buffer);
  }
  stop_time();
  elapsed_time("str_len");

  start_time();
  n = LOOPS;
  while( n-- ) {
    wlen(buffer);
  }
  stop_time();
  elapsed_time("wlen");

  start_time();
  n = LOOPS;
  while( n-- ) {
    slen(buffer);
  }
  stop_time();
  elapsed_time("slen");

  start_time();
  n = LOOPS;
  while( n-- ) {
    xlen(buffer);
  }
  stop_time();
  elapsed_time("xlen");

  return(0);
}


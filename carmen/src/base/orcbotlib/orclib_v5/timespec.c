#include <sys/time.h>
#include <time.h>
#include <stdio.h>

void timespec_now(struct timespec *ts)
{
  struct timeval  tv;

  // get the current time
  gettimeofday(&tv, NULL);
  ts->tv_sec=tv.tv_sec;
  ts->tv_nsec=tv.tv_usec*1000;  
}

void timespec_addms(struct timespec *ts, long ms)
{
  int sec=ms/1000;
  ms=ms-sec*1000;

  // perform the addition
  ts->tv_nsec+=ms*1000000;

  // adjust the time
  ts->tv_sec+=ts->tv_nsec/1000000000 + sec;
  ts->tv_nsec=ts->tv_nsec%1000000000;
}

int timespec_compare(struct timespec *a, struct timespec *b)
{
  if (a->tv_sec!=b->tv_sec)
    return a->tv_sec-b->tv_sec;

  return a->tv_nsec-b->tv_nsec;
}

void timespec_print(struct timespec *a)
{
  printf("%li.%09li\n",a->tv_sec, a->tv_nsec);
}

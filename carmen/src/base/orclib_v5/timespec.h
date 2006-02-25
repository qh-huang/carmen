#ifndef _TIMESPEC_H
#define _TIMESPEC_H

#include <sys/time.h>
#include <time.h>

void timespec_now(struct timespec *ts);
void timespec_addms(struct timespec *ts, long ms);
int timespec_compare(struct timespec *a, struct timespec *b);
void timespec_print(struct timespec *a);

#endif

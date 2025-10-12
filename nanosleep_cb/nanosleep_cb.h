#ifndef NANOSLEEP_CB_H
#define NANOSLEEP_CB_H

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>

typedef void callback_t(void);

typedef struct {
	uint64_t resolution;
	uint64_t interval;
	callback_t *callback;
	int running;
	int quit;
	struct timespec remaining;
	struct timespec request;
	struct timeval now;
	uint64_t present;
	uint64_t goal;
} ncb_timer_rec_t;

void ncb_timer_create(ncb_timer_rec_t *a_ncb_timer_rec);
void ncb_timer_start(ncb_timer_rec_t *a_ncb_timer_rec);
void ncb_timer_stop(ncb_timer_rec_t *a_ncb_timer_rec);
void ncb_timer_destroy(ncb_timer_rec_t *a_ncb_timer_rec);

#endif // NANOSLEEP_CB_H


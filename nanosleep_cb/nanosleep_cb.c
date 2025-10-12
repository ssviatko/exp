#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>

#include "nanosleep_cb.h"

static void *generator_tf(void *arg)
{
	((ncb_timer_rec_t *)arg)->request.tv_sec = 0;
	((ncb_timer_rec_t *)arg)->request.tv_nsec = ((ncb_timer_rec_t *)arg)->resolution * 1000;
	gettimeofday(&((ncb_timer_rec_t *)arg)->now, NULL);
	((ncb_timer_rec_t *)arg)->goal = (((ncb_timer_rec_t *)arg)->now.tv_sec * 1000000) + ((ncb_timer_rec_t *)arg)->now.tv_usec;

	while (1) {
		((ncb_timer_rec_t *)arg)->goal += ((ncb_timer_rec_t *)arg)->interval;
		do {
			if (nanosleep(&((ncb_timer_rec_t *)arg)->request, &((ncb_timer_rec_t *)arg)->remaining) == -1) {
				printf("nanosleep error: %d\n", errno);
			}
			gettimeofday(&((ncb_timer_rec_t *)arg)->now, NULL);
			((ncb_timer_rec_t *)arg)->present = (((ncb_timer_rec_t *)arg)->now.tv_sec * 1000000) + ((ncb_timer_rec_t *)arg)->now.tv_usec;

		} while (((ncb_timer_rec_t *)arg)->present < ((ncb_timer_rec_t *)arg)->goal);
		if (((ncb_timer_rec_t *)arg)->running > 0) {
			(*((ncb_timer_rec_t *)arg)->callback)();
		}
		if (((ncb_timer_rec_t *)arg)->quit > 0)
			return 0;
	}
}

void ncb_timer_create(ncb_timer_rec_t *a_ncb_timer_rec)
{
	pthread_t generator_thread;
	
	a_ncb_timer_rec->running = 0;
	a_ncb_timer_rec->quit = 0;

	pthread_create(&generator_thread, NULL, generator_tf, a_ncb_timer_rec);
	pthread_detach(generator_thread);
}

void ncb_timer_start(ncb_timer_rec_t *a_ncb_timer_rec)
{
	a_ncb_timer_rec->running = 1;
}

void ncb_timer_stop(ncb_timer_rec_t *a_ncb_timer_rec)
{
	a_ncb_timer_rec->running = 0;
}

void ncb_timer_destroy(ncb_timer_rec_t *a_ncb_timer_rec)
{
	a_ncb_timer_rec->quit = 1;
}


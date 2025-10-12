#include <stdio.h>

#include "nanosleep_cb.h"

void timer1_callback()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("timer1 caught on %li.%06li\n", now.tv_sec, now.tv_usec);
}

void timer2_callback()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("timer2 caught on %li.%06li\n", now.tv_sec, now.tv_usec);
}

int main()
{
	ncb_timer_rec_t timer1, timer2;
	
	timer1.resolution = 1000;
	timer1.interval = 300000;
	timer1.callback = timer1_callback;
	ncb_timer_create(&timer1);
	ncb_timer_start(&timer1);
	
	timer2.resolution = 1000;
	timer2.interval = 2000000;
	timer2.callback = timer2_callback;
	ncb_timer_create(&timer2);
	ncb_timer_start(&timer2);
	
	printf("waiting 10 seconds...\n");
	sleep(10);
	ncb_timer_stop(&timer1);
	printf("waiting 4 seconds...\n");
	sleep(4);
	
	ncb_timer_destroy(&timer1);
	ncb_timer_destroy(&timer2);
	
	return 0;
}

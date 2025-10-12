#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_THREADS 128

typedef struct {
	int runflag;
	int thread_num;
	volatile uint64_t workpiece;
} thread_local_t;

thread_local_t thread_local[MAX_THREADS];

void *work_tf(void *arg);

void usage()
{
	printf("usage: work1 <thread_count> (max: %d threads)\n", MAX_THREADS);
}

int main(int argc, char **argv) {
	int res, i, j;
	uint64_t total_iterations, last_total_iterations;
	double iter_sec_avg = 0.0;
	pthread_t work_thread[MAX_THREADS];
	int thread_count;

	if (argc != 2) {
		usage();
		exit(EXIT_FAILURE);
	}

	thread_count = atoi(argv[1]);
	if ((thread_count > MAX_THREADS) || (thread_count < 1)) {
		usage();
		exit(EXIT_FAILURE);
	}
	
	for (i = 0; i < thread_count; ++i) {
		// initialize thread local structure
		thread_local[i].runflag = 1;
		thread_local[i].thread_num = i;
		thread_local[i].workpiece = 0;
		// create thread
		res = pthread_create(&work_thread[i], NULL, work_tf, (void *)&thread_local[i]);
		if (res != 0) {
			fprintf(stderr, "Thread %d creation failed", i);
			exit(EXIT_FAILURE);
		}
	}

	// tabulate results every second
	last_total_iterations = 0;
	for (i = 1; i <= 60; ++i) {
		sleep(1);
		total_iterations = 0;
		for (j = 0; j < thread_count; ++j) {
			total_iterations += thread_local[j].workpiece;
		}
		if (i == 1) {
			iter_sec_avg = (double)total_iterations;
		} else {
			iter_sec_avg = (iter_sec_avg * (double)(i - 1)) + (double)(total_iterations - last_total_iterations);
			iter_sec_avg /= (double)i;
		}
		printf("time +%d secs: total iterations: %ld iterations/sec: %ld avg: %lf\n", i, total_iterations, total_iterations - last_total_iterations, iter_sec_avg);
		last_total_iterations = total_iterations;
	}

	// shut off the threads with the runflags
	for (i = 0; i < thread_count; ++i) {
		thread_local[i].runflag = 0;
	}

	// join the threads
	for (i = 0; i < thread_count; ++i) {
		printf("Waiting for thread %d to finish...\n", i);
		res = pthread_join(work_thread[i], NULL);
		if (res!=0) {
			fprintf(stderr, "Thread %d join failed.", i);
			exit(-1);
		}
	}
	return 0;
}

void iterate()
{
	uint32_t i, j;
	volatile uint32_t a, b, r1;
	volatile uint64_t c, d, r2;
	volatile double w, x, y, z, r3;
	volatile float w1, x1, y1, z1, r4;
	uint8_t buff[32];

	for (i = 0; i < 1000; ++i) {
		// 32 bit integer manipulation
		a = 0xaaaaaaaa;
		b = 0xdeadbeef;
		a &= 0xf0f0f0f0;
		b &= 0xff00ff00;
		a |= b;
		a <<= 2;
		b = a;
		a >>= 4;
		a |= b;
		a ^= 0xffffffff;
		b = a;
		b >>= 3;
		a |= b;
		a += 7;
		b -= 5;
		r1 += a + b;

		// 64 bit integer manipulation
		c = 0xaaaaaaaaaaaaaaaaULL;
		d = 0xdeadbeefc0edbabeULL;
		c &= 0xf0f0f0f0f0f0f0f0ULL;
		d &= 0xff00ff00ff00ff00ULL;
		c |= d;
		c <<= 2;
		d = c;
		c >>= 4;
		c |= d;
		c ^= 0xffffffffffffffffULL;
		d = c;
		d >>= 3;
		c |= d;
		c += 7;
		d -= 5;
		r2 += c + d;

		// floating point mix
		w = 7.84752;
		x = 8.98273;
		y = -2.34585;
		z = -4.58843;

		w += x + y + z;
		x = w / y;
		y = z * w / x;
		z = y * x + w / 5.4441;
		r3 += w - x + y - z;

		w1 = 7.84752;
		x1 = 8.98273;
		y1 = -2.34585;
		z1 = -4.58843;

		w1 += x1 + y1 + z1;
		x1 = w1 / y1;
		y1 = z1 * w1 / x1;
		z1 = y1 * x1 + w1 / 5.4441;
		r4 += w1 - x1 + y1 - z1;

		// memory+decision mix
		for (j = 0; j < 32; ++j) {
			buff[j] = j & 0x000000ff;
			buff[j] += 0x7f;
			if (buff[j] & 0x80) {
				buff[j] -= 0x3e;
			}
			if (buff[j] & 0x40) {
				buff[j] += 0x2b;
			}
			if (buff[j] & 0x20) {
				buff[j] -= 0x1a;
			}
			if (buff[j] & 0x10) {
				buff[j] ^= 0xff;
			}
			if (r4 < 1.0) {
				buff[j] -= 3;
			}
			if (r3 < 1.0) {
				buff[j] += 2;
			}
			if (r2 < 10) {
				buff[j] -= 8;
			}
			if (r1 < 10) {
				buff[j] += 19;
			}
		}
	}
}

void *work_tf(void *arg) {
	thread_local_t *local = arg;
	printf("Work thread %d is running:\n", local->thread_num);
	while (local->runflag > 0) {
		iterate();
		++local->workpiece;
	}
	pthread_exit(NULL);
}


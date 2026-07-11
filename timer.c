#include "timer.h"
#include "globals.h"
#include <bits/time.h>
#include <bits/types/struct_itimerspec.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

void init_timer(const unsigned int ms) {
	struct itimerspec tv = (struct itimerspec){0};
	timerfd = timerfd_create(CLOCK_MONOTONIC, 0);

	if(timerfd < 0)
		goto _init_timer_error;

	tv.it_value.tv_sec    = 0;
	tv.it_value.tv_nsec   = ms * 100 * 100 * 100;
	tv.it_interval.tv_sec  = 0;
	tv.it_interval.tv_nsec = ms * 100 * 100 * 100;

	timerfd_settime(timerfd, 0, &tv, NULL);
	return;

_init_timer_error:
	setbuf(stderr, NULL);
	fprintf(stderr, "WARNING: timerfd error. Using usleep for timer interval management\n");
}

void timer_wait(const unsigned int ms) {
	if(timerfd < 0)
		usleep(ms * 1000);
	else
		read(timerfd, NULL, sizeof(uint64_t));
}

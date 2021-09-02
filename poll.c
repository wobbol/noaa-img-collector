#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "url.h"
#include "ctx.h"
#include "data_source.h"

static timer_t create_thread_timer(struct ctx *c, void(*thread_function)(union sigval))
{
	clockid_t c_id = CLOCK_MONOTONIC;
	struct sigevent evp = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_value = {.sival_ptr = c},
		.sigev_notify_function = thread_function,
	};
	timer_t t_id;
	timer_create(c_id, &evp, &t_id);
	return t_id;
}
static void start_recuring_thread(timer_t t_id, time_t interval)
{
	int flags = TIMER_ABSTIME;
	struct itimerspec val = {
		.it_value = {.tv_sec = 1}, /* with TIMER_ABSTIME this means ASAP. */
		.it_interval = {.tv_sec = interval, },
	};
	struct itimerspec oval;

	timer_settime(t_id, flags, &val, &oval);
}

int poll_url_noret(struct ctx *c, void(*thread_function)(union sigval))
{
	timer_t t_id = create_thread_timer(c, thread_function);
	time_t min5 = 5 * 60;
	start_recuring_thread(t_id, min5);
	for(;;) { /* keep alive to continue spawning threads */
		sleep(60);
	}
	return 0;
}

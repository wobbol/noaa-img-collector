#include <assert.h>
#include <curl/curl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

#include "curl.h"
#include "url.h"
#include "data_source.h"

#define MAX_URL_SZ 256

struct ctx {
	char url[MAX_URL_SZ];
	FILE *err;
	struct data_source *d;
};

static int file_exists(char *fname)
{
	return access( fname, F_OK ) == 0;
}

int resume(time_t start, struct data_source *i)
{
	while(start < time(NULL)) {
		char *name = url_get_filename(i, start);
		char *url = url_get_past(i, name, start);
		start += 5 * 60;
			printf("checking %s\n", name);
			fflush(stdout);
		if(!file_exists(name)) {
			printf("getting %s\n", name);
			fflush(stdout);
			FILE *out = fopen(name, "w");
			int err = do_curl(url, out, stdout);
			fclose(out);
			if(err) {
				remove(name);
			}
		}
	}
	return 0;
}

void thread_poll_current(union sigval sv)
{
	struct ctx *c = sv.sival_ptr;
	fprintf(c->err, "doing the thing\n");
	fflush(c->err);

	FILE *output = fopen(url_get_filename(c->d, time(NULL)), "w");

	do_curl(c->url, output, c->err);

	fclose(output);
}
timer_t create_thread_timer(struct ctx *c)
{
	clockid_t c_id = CLOCK_MONOTONIC;
	struct sigevent evp = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_value = {.sival_ptr = c},
		.sigev_notify_function = thread_poll_current,
	};
	timer_t t_id;
	timer_create(c_id, &evp, &t_id);
	return t_id;
}
void start_recuring_thread(timer_t t_id, time_t interval)
{
	int flags = TIMER_ABSTIME;
	struct itimerspec val = {
		.it_value = {.tv_sec = 1}, /* with TIMER_ABSTIME this means ASAP. */
		.it_interval = {.tv_sec = interval, },
	};
	struct itimerspec oval;

	timer_settime(t_id, flags, &val, &oval);
}

void init_ctx(struct ctx *c, struct data_source *d)
{
	c->err = stdout;
	snprintf(c->url, sizeof(c->url), "%s/%s/ABI/CONUS/%d/%s.jpg",
			d->urlbase, d->satalite, d->band, d->size);
	c->d = d;
	//c->url = "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/416x250.jpg",
	//c->url = "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/5000x3000.jpg",
}
int noret_poll_url(struct ctx *c)
{
	timer_t t_id = create_thread_timer(c);
	time_t min5 = 5 * 60;
	start_recuring_thread(t_id, min5);
	for(;;) { /* keep alive to continue spawning threads */
		sleep(60);
	}
	return 0;
}
int main(int argc, char *argv[])
{
	struct data_source i = {
		.urlbase = "https://cdn.star.nesdis.noaa.gov",
		.band = 13,
		.satalite = "GOES16",
		.size = "416x250",
	};
	if(argc > 1) {
		resume(atoll(argv[1]), &i);
	}
	printf("done resumeing\n");
	struct ctx c;
	init_ctx(&c, &i);
	noret_poll_url(&c);
	return 0;
}

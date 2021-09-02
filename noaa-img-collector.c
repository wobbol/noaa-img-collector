#include <assert.h>
#include <curl/curl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

#include "curl.h"

#define MAX_URL_SZ 256

struct data_source {
	char urlbase[MAX_URL_SZ];
	char satalite[32];
	char size[32];
	int band;
};

struct ctx {
	char url[MAX_URL_SZ];
	FILE *err;
	struct data_source *d;
};

void on_the_1s_and_6s(struct tm *t)
{
	while(t->tm_min % 10 != 1 && t->tm_min % 10 != 6) {
		if(t->tm_min == 0) {
			t->tm_hour--;
			t->tm_min = 60;
		}
		if(t->tm_hour < 0) {
			t->tm_hour = 23;
			t->tm_yday--;
		}
		if(t->tm_yday < 0) {
			t->tm_yday = 365;
		}
		t->tm_min--;
	}
}

char *get_date_time(time_t time)
{
	static char ret[32];
	struct tm *t = gmtime(&time);

	on_the_1s_and_6s(t);
	/* there are never any images released at 56 mins */
	if(t->tm_min % 100 == 56) {
		t->tm_min--;
	}
	on_the_1s_and_6s(t);
	//snprintf(ret, sizeof(ret), "%4.4d-%3.3d-%2.2d-%2.2d",
	snprintf(ret, sizeof(ret), "%4.4d%3.3d%2.2d%2.2d",
			t->tm_year + 1900, t->tm_yday + 1,
			t->tm_hour, t->tm_min);
	return ret;
}
char *get_filename(struct data_source *d, time_t time)
{
	static char ret[128];
	char *date_time = get_date_time(time);

	// 20212421346_GOES16-ABI-CONUS-13-416x250.jpg
	// yyyydddnnnn_SSSSSS-ABI-CONUS-bb-zzzzzzz.jpg

	snprintf(ret, sizeof(ret),"%s_%s-ABI-CONUS-%d-%s.jpg",
			date_time, d->satalite, d->band, d->size);
	return ret;
}
char *get_past_url(struct data_source *d, char *filename, time_t time)
{
	static char u[MAX_URL_SZ];

	snprintf(u, sizeof(u),"%s/%s/ABI/CONUS/%d/%s",
			d->urlbase, d->satalite, d->band, filename);
	return u;
}

void thread_poll_current(union sigval sv)
{
	struct ctx *c = sv.sival_ptr;
	fprintf(c->err, "doing the thing\n");
	fflush(c->err);

	FILE *output = fopen(get_filename(c->d, time(NULL)), "w");

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
int file_exists(char *fname)
{
	return access( fname, F_OK ) == 0;
}
int resume(time_t start, struct data_source *i)
{
	while(start < time(NULL)) {
		char *name = get_filename(i, start);
		char *url = get_past_url(i, name, start);
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

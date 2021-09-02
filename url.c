#include <stdlib.h>
#include <time.h>

#include "curl.h"
#include "data_source.h"

static void on_the_1s_and_6s(struct tm *t)
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

static char *get_date_time(time_t time)
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
char *url_get_filename(struct data_source *d, time_t time)
{
	static char ret[128];
	char *date_time = get_date_time(time);

	// 20212421346_GOES16-ABI-CONUS-13-416x250.jpg
	// yyyydddnnnn_SSSSSS-ABI-CONUS-bb-zzzzzzz.jpg

	snprintf(ret, sizeof(ret),"%s_%s-ABI-CONUS-%d-%s.jpg",
			date_time, d->satalite, d->band, d->size);
	return ret;
}
char *url_get_past(struct data_source *d, char *filename, time_t time)
{
	static char u[256];

	snprintf(u, sizeof(u),"%s/%s/ABI/CONUS/%d/%s",
			d->urlbase, d->satalite, d->band, filename);
	return u;
}

char *url_get_current(struct data_source *d)
{
	static char u[256];
	// https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/5000x3000.jpg
	snprintf(u, sizeof(u), "%s/%s/ABI/CONUS/%d/%s.jpg",
			d->urlbase, d->satalite, d->band, d->size);
	return u;
}

#include <assert.h>
#include <curl/curl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

#define MAX_URL_SZ 256

struct ctx {
	char url[MAX_URL_SZ];
	FILE *err;
};

size_t handle_curl_data(const unsigned char *ptr, size_t size, size_t nmemb, void *userdata)
{
	assert(size == 1);
	FILE *out = userdata;
	return fwrite(ptr, size, nmemb, out);
}
char *get_date_time(time_t now)
{
	static char ret[32];
	struct tm *t = gmtime(&now);

	while(t->tm_min % 10 != 1 && t->tm_min % 10 != 6) {
		if(t->tm_min == 0) {
			t->tm_hour--;
		}
		t->tm_min--;
	}
	/* there are never any images released at 56 mins */
	if(t->tm_min % 100 == 56) {
		t->tm_min--;
	}
	while(t->tm_min % 10 != 1 && t->tm_min % 10 != 6) {
		if(t->tm_min == 0) {
			t->tm_hour--;
		}
		t->tm_min--;
	}
	//snprintf(ret, sizeof(ret), "%4.4d-%3.3d-%2.2d-%2.2d",
	snprintf(ret, sizeof(ret), "%4.4d%3.3d%2.2d%2.2d",
			t->tm_year + 1900, t->tm_yday + 1,
			t->tm_hour, t->tm_min);
	return ret;
}
char *get_filename(time_t now)
{
	static char ret[64];
	char *date_time = get_date_time(now);
	char sat[] = "GOES16";
	char size[] = "416x250";
	int band = 13;

	// 20212421346_GOES16-ABI-CONUS-13-416x250.jpg
	// yyyydddnnnn_SSSSSS-ABI-CONUS-bb-zzzzzzz.jpg

	snprintf(ret, sizeof(ret),"%s_%s-ABI-CONUS-%d-%s.jpg",
			date_time, sat, band, size);
	return ret;
}
char *get_past_url(char *urlbase, time_t now)
{
	static char u[MAX_URL_SZ];
	char *filename = get_filename(now);
	char sat[] = "GOES16";
	char size[] = "416x250";
	int band = 13;

	snprintf(u, sizeof(u),"%s/%s/ABI/CONUS/13/%s",
			urlbase, sat, filename);
	return u;
}

void do_curl(char *url, char *pathname, FILE* err)
{
	FILE *output = fopen(pathname, "w");


	//do the curl thing
	CURL *curl;
	CURLcode curl_res;
	char curl_errorbuf[CURL_ERROR_SIZE];
	if(!(curl = curl_easy_init())) {
		fprintf(err, "could not init curl\n");
		fflush(err);
		return;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "stormwatch/v0.0.0");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_curl_data);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errorbuf);
	curl_res = curl_easy_perform(curl);
	if(curl_res) {
		fprintf(err, "%s\n", curl_errorbuf);
		fflush(err);
		return;
	}
	curl_easy_cleanup(curl);
	fclose(output);
}
void thread_function(union sigval sv)
{
	struct ctx *c = sv.sival_ptr;
	fprintf(c->err, "doing the thing\n");
	//fflush(c->err);

	int now = (int)time(NULL);
	char *filename = get_filename(now);

	//do the curl thing
	do_curl(c->url, filename, c->err);
}

int poll_url_5_min()
{
	clockid_t c_id = CLOCK_MONOTONIC;
	struct ctx c = {
		.err = stdout,
		.url =  "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/416x250.jpg",
		//.url =  "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/5000x3000.jpg",
	};
	struct sigevent evp = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_value = {.sival_ptr = &c},
		.sigev_notify_function = thread_function,
	};
	timer_t t_id;
	timer_create(c_id, &evp, &t_id);
	int flags = TIMER_ABSTIME;
	//int flags = 0;
	struct timespec time_1 = { .tv_sec = 1, };
	struct timespec time_5 = { .tv_sec = 5 * 60, };
	struct itimerspec val = {
		.it_value = time_1,
		.it_interval = time_5,
	};
	struct itimerspec oval;

	timer_settime(t_id, flags, &val, &oval);
	for(;;) {
		sleep(60);
	}
	return 0;
}
int file_exists(char *fname)
{
	return access( fname, F_OK ) == 0;
}
int resume(time_t start)
{

	char urlbase[] = "https://cdn.star.nesdis.noaa.gov/";
	while(start < time(NULL)) {
		char *url = get_past_url(urlbase, start);
		char *name = get_filename(start);
		start += 5 * 60;
		if(!file_exists(name)) {
			do_curl(url, name, stdout);
		}
	}
	return 0;
}
int main(int argc, char *argv[])
{
	if(argc > 1) {
		resume(atoll(argv[1]));
	}
	poll_url_5_min();
	return 0;
}

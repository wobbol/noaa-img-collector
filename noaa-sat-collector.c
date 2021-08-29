#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include <assert.h>

int counter = 0;
size_t handle_curl_data(const unsigned char *ptr, size_t size, size_t nmemb, void *userdata)
{
	assert(size == 1);
	FILE *out = userdata;
	return fwrite(ptr, size, nmemb, out);
}
void thread_function(union sigval sv)
{
	printf("doing the thing\n");
	fflush(stdout);
	char filename[32];
	time_t now = time(NULL);
	sprintf(filename,"%d.jpg",now);
	FILE *output = fopen(filename, "w");

	//do the curl thing
	CURL *curl;
	CURLcode curl_res;
	char curl_errorbuf[CURL_ERROR_SIZE];
	if(!(curl = curl_easy_init())) {
		printf("could not init curl\n");
		fflush(stdout);
		return;
	}
	curl_easy_setopt(curl, CURLOPT_URL, "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/416x250.jpg");
	//curl_easy_setopt(curl, CURLOPT_URL, "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/CONUS/13/5000x3000.jpg");
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "stormwatch/v0.0.0");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_curl_data);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errorbuf);
	curl_res = curl_easy_perform(curl);
	if(curl_res) {
		printf("%s\n", curl_errorbuf);
		fflush(stdout);
		return;
	}
	curl_easy_cleanup(curl);
	
}

int main()
{
	clockid_t c_id = CLOCK_MONOTONIC;
	struct sigevent evp = {
		.sigev_notify = SIGEV_THREAD,
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

#include <assert.h>
#include <curl/curl.h>
#include <stdio.h>

static size_t handle_curl_data(const unsigned char *ptr, size_t size, size_t nmemb, void *userdata)
{
	assert(size == 1);
	FILE *out = userdata;
	return fwrite(ptr, size, nmemb, out);
}

int do_curl(char *url, FILE *out, FILE* err)
{

	CURL *curl;
	CURLcode curl_res;
	char curl_errorbuf[CURL_ERROR_SIZE];
	if(!(curl = curl_easy_init())) {
		fprintf(err, "could not init curl\n");
		fflush(err);
		return 1;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "noaa-img-collector/v0.0.0");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_curl_data);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errorbuf);
	curl_res = curl_easy_perform(curl);
	if(curl_res) {
		fprintf(err, "%s\n", curl_errorbuf);
		fflush(err);
		return 1;
	}
	curl_easy_cleanup(curl);
	return 0;
}

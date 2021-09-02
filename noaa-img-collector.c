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
#include "poll.h"
#include "ctx.h"
#include "data_source.h"

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
	poll_url_noret(&c, thread_poll_current);
	return 0;
}

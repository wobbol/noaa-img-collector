noaa-img-collector: noaa-img-collector.c curl.c url.c poll.c ctx.c
	$(CC) -o noaa-img-collector noaa-img-collector.c curl.c url.c poll.c ctx.c -lrt -lcurl

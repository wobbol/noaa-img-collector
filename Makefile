noaa-img-collector: noaa-img-collector.c curl.c url.c
	$(CC) -o noaa-img-collector noaa-img-collector.c curl.c url.c -lrt -lcurl

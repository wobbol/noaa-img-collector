noaa-img-collector: noaa-img-collector.c curl.c
	$(CC) -o noaa-img-collector noaa-img-collector.c curl.c -lrt -lcurl

noaa-sat-collector: noaa-sat-collector.c
	$(CC) -o noaa-sat-collector noaa-sat-collector.c -lrt -lcurl

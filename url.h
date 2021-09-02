#ifndef url_h
#define url_h

#include <time.h>

#include "data_source.h"

char *url_get_filename(struct data_source *d, time_t time);
char *url_get_past(struct data_source *d, char *filename, time_t time);
#endif

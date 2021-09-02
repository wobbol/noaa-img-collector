#ifndef ctx_h
#define ctx_h

#include <stdio.h>
#include "data_source.h"

struct ctx {
	char url[256];
	FILE *err;
	struct data_source *d;
};

void init_ctx(struct ctx *c, struct data_source *d);
#endif

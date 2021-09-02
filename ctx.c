#include <string.h>

#include "ctx.h"
#include "data_source.h"
#include "url.h"

void init_ctx(struct ctx *c, struct data_source *d)
{
	c->err = stdout;
	c->d = d;
	strncpy(c->url, url_get_current(d), sizeof(c->url));
}

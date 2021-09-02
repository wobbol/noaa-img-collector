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
#include "ctx.h"
#include "data_source.h"

int poll_url_noret(struct ctx *c, void(*thread_function)(union sigval));

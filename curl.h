#ifndef curl_h
#define curl_h
#include <stdio.h>

/*
 * puts the contents of url into out.
 * returns non zero exit on error.
 */
int do_curl(char *url, FILE *out, FILE* err);
#endif

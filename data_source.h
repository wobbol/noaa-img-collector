#ifndef data_source_h
#define data_source_h
struct data_source {
	char urlbase[256];
	char satalite[32];
	char size[32];
	int band;
};
#endif

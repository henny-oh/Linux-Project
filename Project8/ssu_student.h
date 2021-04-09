#ifndef SSUSTUDENT_H__
#define SSUSTUDENT_H__

#include "ssu_file.h"
#include "ssu_string.h"

#define S_NUM 10 

struct ssu_student{
	char s_num[S_NUM];
	float score[MAX_PRBL];
	float totalscore;
	char path[PATH_SIZE];
	struct ssu_file buf_file;
	struct token *headtoken;
	struct ssu_student *next;
};

extern struct ssu_student *students;

int getallstd(const char *stdpath);
struct ssu_student *getprevstd(struct ssu_student *std);
struct ssu_student *findstd(const char *stdnum);

#endif

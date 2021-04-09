#ifndef SSUFILE_H__
#define SSUFILE_H__

#include <stdio.h>
#include "ssu_string.h"

#define FNAME_SIZE 10
#define MAX_PRBL 100
#define PATH_SIZE 100
#define T_FILE 0
#define C_FILE 1

struct ssu_file{
	char f_name[FNAME_SIZE];
	int p_num;
	int sub_num;
	int f_num;
	float score;
	char path[PATH_SIZE];
	char *text;
	struct ssu_file *next;
	struct token *head;
};

extern struct ssu_file *ansfile;

int getans(const char *anspath);
int readsfile(const char *fname, struct ssu_file *s_file);
struct ssu_file *getprevans(struct ssu_file *ans);
struct ssu_file *findans(const char *ansnum);
void savestable(FILE *s_table, int type);
void loadstable(FILE *s_table);
void savestdscore(FILE *s_score);

#endif

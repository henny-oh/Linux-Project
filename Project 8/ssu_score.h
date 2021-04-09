#ifndef SSUSCORE_H__
#define SSUSCORE_H__

#include "ssu_string.h"
#include "ssu_file.h"

#define A_RIGHT 1
#define A_WRONG 0
#define C_WARNING_SCORE 0.1
#define C_ERROR_SCORE 0
#define C_TIMEOVER_SCORE 0

float tscoring(struct token *anshead, struct token *stdhead, const float score);
float cscoring(struct ssu_file *prog,struct ssu_file *ans);
char *compileprog(const char *filename);
char *execprog(const char *filename);
void scoring(const char *stdpath);
float stdscoring(const char *stdnum);
void anscompile();
int warncount(const char *pathname);

#endif

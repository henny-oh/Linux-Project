#ifndef SSUSTRING_H__
#define SSUSTRING_H__

#define BUF_SIZE 1024
#define OTH_ANS 100
#define PRE_OP -1
#define POST_OP 1
#define TWO_OP 2

struct token{
	char str[100];
	struct token *nexttoken;
	int operands;
	int ch;
	int isch;
};

extern char buf[BUF_SIZE];
extern struct token *anshead[];

struct token *str2token(const char *str, struct token *head);
int isopstr(const char *str);
int isop(char c);
int isblk(char c);
void printtokens(const struct token *head);
int tokencmp(struct token *anshead, struct token *stdhead);
struct token *nextans(struct token *anstoken);
int stringcmp(const char *anstext, const char *stdtext);
char *strlwr(const char *str);
struct token *postfix(struct token *head);
int priority(struct token *tk);
struct token *poptoken(struct token *head);
void pushtoken(struct token *head, struct token *push);
void inserttoken(struct token *head, struct token *push);
void makepostfix(struct token *head, struct token *postfixhead, struct token *op);
struct token *sorttoken(struct token *head);
int checktype(const char *str);
#endif

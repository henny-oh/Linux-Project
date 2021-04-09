#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ssu_string.h"
#include "ssu_file.h"

char buf[BUF_SIZE];
struct token *anshead[MAX_PRBL];

// string을 토큰 리스트로 만들어 리턴
struct token *str2token(const char *str, struct token *head){
	struct token *tmp_token = NULL;
	struct token *pre_token = head;
	struct token *tmp = NULL;
	char c;
	int i;

	if(head != NULL){
		//free
	}
	else{
		// 헤드가 널일 경우 헤드 메모리 할당
		head = (struct token*) calloc(1, sizeof(struct token));
	}
	
	pre_token = head;
	// string 을 처음 부터 하나씩 읽어서 구분
	while( (c = *str) != 0 ){
		// blank 구분
		if( isblk(c) >= 1 ){
			str++;
			continue;
		}
		// 맨뒤에 있는 개행은 널 문자로 변경
		if( c == '\n' ){
			if( *(str+1) == 0 ){
				str++;
				continue;
			}
		}

		
		tmp_token = (struct token*) calloc(1, sizeof(struct token));
		pre_token->nexttoken = tmp_token;
		tmp_token->nexttoken = NULL;
		// 연산자 괄호 쉼표 등을 구분
		if( c == '\n' ){
			tmp_token->str[0] = c;
			tmp_token->str[1] = '\0';
		}

		else if( c == '(' || c == ')'){
			tmp_token->str[0] = c;
			tmp_token->str[1] = '\0';
		}
		else if( c == '[' || c == ']'){
			tmp_token->str[0] = c;
			tmp_token->str[1] = '\0';
		}
		else if( c == ',' ){
			tmp_token->str[0] = c;
			tmp_token->str[1] = '\0';
		}
		else if( c == '\"' ){
			tmp_token->str[0] = c;
			str++;
			i = 1;
			while( (c = *str) != 0 ){
				if( c == '\"'){
					tmp_token->str[i] = c;
					break;
				}
				tmp_token->str[i] = c;
				i++;
				str++;
			}
		}
		else if( isop(c) > 0 ){
			int max = 2;
			// <<= >>= 연산자 처리
			if( c == '>' || c == '<'){
				if( c == *(str+1) ){
					if( *(str+2) == '=' ){
						max = 3;
					}
				}
			}

			for(i=0; i<max; i++){
				tmp_token->str[i] = c;
				c = *(str+1);
				int ret = isop(c);
				if( ret >= 4 || ret <= 0 || isblk(c) >= 1 || i == max-1 || c=='\n' ){
					tmp_token->str[i+1] = '\0';
					// 붙어 있는 연산자(&& || != 등) 만날 경우에만 붙여서 처리
					// 이외에 &^ 이런 문구는 제외
					if( isopstr(tmp_token->str) != 1 && tmp_token->str[0] != ':'){
						tmp = (struct token *) calloc(1, sizeof(struct token));
						tmp->str[0] = tmp_token->str[0];
						tmp->str[1] = 0;
						tmp_token->str[0] = tmp_token->str[1];
						tmp_token->str[1] = 0;
						pre_token->nexttoken = tmp;
						tmp->nexttoken = tmp_token;
					}
					break;
				}
				else{
					str++;
				}
			}
		}
		else{
			i = 0;
			// 연산자 등이 아니라 변수 값을 만났을 경우에 공백을 만날 때까지 저장
			while( c != ' ' ){
				tmp_token->str[i] = c;
				i++;
				c = *(str+1);
				if( isop(c) > 0 || isblk(c) >= 1 || c == '\n'){
					tmp_token->str[i] = '\0';
					break;
				}
				str++;
			}
		}

		str++;
		pre_token = tmp_token;
	}
	postfix(head);
	return head;
}

// 연산자 인지 확인하는 함수
int isopstr(const char *str){
	int ret = 0;

	if( strcmp(str, "++") == 0 || strcmp(str, "--") == 0){
		ret = 1;
	}
	else if( strcmp(str, "!") == 0 || strcmp(str, "~") == 0){
		ret = 1;
	}
	else if( strcmp(str, "->") == 0 || strcmp(str, ".") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "*") == 0 || strcmp(str, "*=") == 0){
		ret = 1;
	}
	else if( strcmp(str, "/") == 0 || strcmp(str, "/=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "%") == 0 || strcmp(str, "%=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "+") == 0 || strcmp(str, "+=") == 0){
		ret = 1;
	}
	else if( strcmp(str, "-") == 0 || strcmp(str, "-=") == 0){
		ret = 1;
	}
	else if( strcmp(str, "<<") == 0 || strcmp(str, "<<=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, ">>") == 0 || strcmp(str, ">>=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "<") == 0 || strcmp(str, "<=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, ">") == 0 || strcmp(str, ">=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "==") == 0 || strcmp(str, "!=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "&") == 0 || strcmp(str, "&=") == 0){
		ret = 1;
	}
	else if( strcmp(str, "^") == 0 || strcmp(str, "^=") == 0){
		ret = 1;
	}
	else if( strcmp(str, "|") == 0 || strcmp(str, "|=") == 0){
		ret = 1;
	}
	else if( strcmp(str, "&&") == 0 || strcmp(str, "||") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, "=") == 0 ){
		ret = 1;
	}
	else if( strcmp(str, ":") == 0 ){
		ret = 2;
	}
	return ret;
}


// 해당 char가 연산자에 속할 수 있는 지 확인하는 함수
int isop(char c){
	int ret;
	switch(c){
		case '*': case '+': case '-': case '/': case '%':
			ret = 1;
			break;
		case '=': case '!': case '>': case '<':
			ret = 2;
			break;
		case '&': case '|': case '^': case '~':
			ret = 3;
			break;
		case '.': case ',':
			ret = 4;
			break;
		case ':':
			ret = 5;
			break;
		case '(': case '[': 
			ret = 6;
			break;
		case ')': case ']':
			ret = 7;
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

// 공백을 확인하는 함수
int isblk(char c){
	int ret = 0;
	switch(c){
		case ' ': case '\t':
			ret = 1;
			break;
		case '\0':
			ret = 2;
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

void printtokens(const struct token *head){
	struct token *next = NULL;

	if( head != NULL ){
		next = head->nexttoken;
	}

	while( next != NULL ){
		if( next->str[0] != ':'){
			printf("%s ", next->str);
		}
		else{
			printf("\n");
		}
		next = next->nexttoken;
	}
	printf("\n");
}

// 토큰을 비교해서 일치하면 0을 리턴
int tokencmp(struct token *anshead, struct token *stdhead){
	struct token *ansnext = NULL;
	struct token *stdnext = NULL;
	int check = 0;

	if( anshead == NULL && stdhead == NULL){
		return -1;
	}
	ansnext = anshead->nexttoken;
	stdnext = stdhead->nexttoken;

	while( ansnext != NULL && stdnext != NULL){
		if( (check = strcmp(ansnext->str, stdnext->str)) != 0 ){
			if( (ansnext = nextans(ansnext)) == NULL){
				break;
			}
			stdnext = stdhead->nexttoken;
			check = 0;
			continue;
		}
		ansnext = ansnext->nexttoken;
		stdnext = stdnext->nexttoken;
		if(ansnext == NULL){
			break;
		}
		if( ansnext->str[0] == ':' ){
			if( check == 0 ){
				break;
			}
			ansnext = ansnext->nexttoken;
			stdnext = stdhead->nexttoken;	
		}
	}
	return check;
}

// : 있는 경우에 다음 정답으로 이동하여 해당 토큰 처음 포인터를 리턴
struct token *nextans(struct token *anstoken){
	struct token *next = NULL;

	if( anstoken != NULL ){
		next = anstoken->nexttoken;
	}

	while( next != NULL ){
		if( next->str[0] == ':'){
			return next->nexttoken;
		}
		next = next->nexttoken;
	}
	return next;
}

// string을 소문자로 변경하고 공백을 제거해서 비교 같으면 0을 리턴
int stringcmp(const char *anstext, const char *stdtext){
	char *anstextlwr;
	char *stdtextlwr;
	char ansc;
	char stdc;
	int ret = 0;

	if( anstext == NULL || stdtext == NULL ){
		return -1;
	}
	anstextlwr = strlwr(anstext);
	stdtextlwr = strlwr(stdtext);

	if( anstextlwr == NULL || stdtextlwr == NULL ){
		return -1;
	}

	while( (ansc = *anstextlwr) != 0 ){
		if( stdtextlwr == NULL ){
			ret = -1;
			break;
		}
		stdc = *stdtextlwr;
		if( isblk(ansc) > 0 ){
			anstextlwr++;
			ansc = *anstextlwr;
		}
		if( isblk(stdc) > 0 ){
			stdtextlwr++;
			stdc = *stdtextlwr;
		}
		if( ansc == stdc ){
			anstextlwr++;
			ansc = *anstextlwr;
			stdtextlwr++;
			stdc = *stdtextlwr;
		}	
		else{
			ret = -1;
			break;
		}
	}
	//free(anstextlwr);
	//free(stdtextlwr);
	return ret;
}

// stirng 전체를 소문자로 바꾸는 함수
char *strlwr(const char *str){
	char *retstring = (char *) calloc(strlen(str)+1, sizeof(char));
	char *string = retstring;

	strcpy(retstring, str);

	while( *string != 0 ){
		*string = tolower(*string);
		string++;
	}
	return retstring;
}

// 토큰을 후위 표기법으로 바꾸는 함수
struct token *postfix(struct token *head){
	struct token *postfixhead;
	struct token *opstack;
	struct token *next = head;
	struct token *postfixnext;
	struct token *tmp;
	struct token *pretmp;
	char endchar;
	int preop = 0;
	int cnt = 0;
	head->str[0] = 0;
	
	while( next->nexttoken != NULL){
		if( strcmp(next->nexttoken->str, "#include") == 0 ){
			tmp = next->nexttoken->nexttoken;
			while( tmp != NULL ){
				strcat(next->nexttoken->str, tmp->str);
				pretmp = tmp;
				tmp = tmp->nexttoken;
				next->nexttoken->nexttoken = tmp;
				free(pretmp);
			}
			break;
		}
		//no operater
		if( isop(next->nexttoken->str[0]) <= 0){
			preop = 0;
		}
		// (, ), [, ]
		else if(isop(next->nexttoken->str[0]) == 6){
			if( (preop == 0 && next != head) || isop(next->str[0]) == 7 ){
				cnt = 0;
				if(next->nexttoken->str[0] == '(' ){
					endchar = ')';
				}
				else{
					endchar = ']';
				}
				tmp = next->nexttoken;
				while( tmp != NULL ){
					if( tmp->str[0] == endchar ){
						strcat(next->str, tmp->str);
						pretmp = tmp;
						tmp = tmp->nexttoken;
						next->nexttoken = tmp;
						free(pretmp);
						if( cnt > 1 ){
							cnt--;
							continue;
						}
						cnt=0;
						break;
					}
					if( isop(tmp->str[0]) == 6 ){
						cnt++;
					}
					strcat(next->str, tmp->str );
					pretmp = tmp;
					tmp = tmp->nexttoken;
					next->nexttoken = tmp;
					free(pretmp);
				}
				preop = 0;
				continue;
			}
			preop = 0;
		}
		// operater
		else{
			tmp = next->nexttoken->nexttoken;
			//if next is end or ), ] , it must be post operater
			if( strcmp(next->nexttoken->str, "!") == 0 || strcmp(next->nexttoken->str, "~") == 0 ){
				next->nexttoken->operands = PRE_OP;
			}
			else if( strcmp(next->nexttoken->str, "++")==0 || strcmp(next->nexttoken->str, "--")==0){
				if( preop == 0 ){
					next->nexttoken->operands = POST_OP;
				}
				else{
					next->nexttoken->operands = PRE_OP;
				}
			}
			else if( strcmp(next->nexttoken->str, "*") == 0 || strcmp(next->nexttoken->str, "&") == 0  ){
				if( preop != 0 && (next->operands != POST_OP) )
					next->nexttoken->operands = PRE_OP;
				else if( checktype(next->str) == 1 )
					next->nexttoken->operands = PRE_OP;
				else
					next->nexttoken->operands = TWO_OP;
			}
			else if( strcmp(next->nexttoken->str, "+") == 0 || strcmp(next->nexttoken->str, "-") == 0){
				if( preop != 0 || (next->operands != POST_OP)){
					next->nexttoken->operands = PRE_OP;
				}
				else{
					next->nexttoken->operands = TWO_OP;
				}
			}
			else{
				next->nexttoken->operands = TWO_OP;
			}
			preop = 1;
		}
		next=next->nexttoken;
	}
	
	postfixhead = (struct token *) calloc(1, sizeof(struct token));
	opstack = (struct token *) calloc(1, sizeof(struct token));
	makepostfix(head, postfixhead, opstack); 	
	head->nexttoken = postfixhead->nexttoken;
	sorttoken(head);
	//sorttoken(head);
	//printtokens(head);
}
// 연산자 우선순위 값을 리턴
int priority(struct token *tk){
	int ret = 0;

	if( tk->str == NULL ) {
		return 0;
	}
	tk->ch = 0;
	tk->isch = 0;

	if(tk->operands == POST_OP){
		ret = 1;
	}
	else if( tk->operands == PRE_OP ){
		ret = 2;
	}
	else if( tk->operands == TWO_OP ){
		if( strcmp(tk->str, "->") == 0 || strcmp(tk->str, ".") == 0 ){
			ret = 1;
		}
		else if( strcmp(tk->str, "*") == 0 ){
			tk->ch = 1;
			ret = 3;
		}
		else if( strcmp(tk->str, "/") == 0 || strcmp(tk->str, "%") == 0){
			ret = 3;
		}
		else if( strcmp(tk->str, "+") == 0 ){
			tk->ch = 1;
			ret = 4;
		}
		else if( strcmp(tk->str, "-") == 0 ){
			ret = 4;
		}
		else if( strcmp(tk->str, "<<") == 0 || strcmp(tk->str, ">>") == 0 ){
			ret = 5;
		}
		else if( strcmp(tk->str, "<") == 0 || strcmp(tk->str, "<=") == 0 ){
			tk->ch = 1;
			ret = 6;
		}
		else if( strcmp(tk->str, ">") == 0 || strcmp(tk->str, ">=") == 0 ){
			tk->ch = 1;
			ret = 6;
		}
		else if( strcmp(tk->str, "==") == 0 ){
			tk->ch = 1;
			ret = 7;
		}
		else if( strcmp(tk->str, "!=") == 0 ){
			ret = 7;
		}
		else if( strcmp(tk->str, "&") == 0 ){
			tk->ch = 1;
			ret = 8;
		}
		else if( strcmp(tk->str, "^") == 0 ){
			tk->ch = 1;
			ret = 9;
		}
		else if( strcmp(tk->str, "|") == 0 ){
			tk->ch = 1;
			ret = 10;
		}
		else if( strcmp(tk->str, "&&") == 0 ){
			tk->ch = 1;
			ret = 11;
		}
		else if( strcmp(tk->str, "||") == 0 ){
			tk->ch = 1;
			ret = 12;
		}
		else{
			ret = 14;
		}
	}
	else{
		if( strcmp(tk->str, ",") == 0 ){
			tk->ch = 1;
			ret = 15;
		}
		else{
			tk->ch = 0;
			ret = 0;
		}
	}
	return ret;
}

// 스택에서 맨위를 pop
struct token *poptoken(struct token *head){
	struct token *tmp = NULL;

	tmp = head->nexttoken;

	if( head->nexttoken != NULL ){
		head->nexttoken = tmp->nexttoken;
		tmp->nexttoken = NULL;
	}
	return tmp;
}

//맨위에 push 
void pushtoken(struct token *head, struct token *push){
	struct token *tmp;

	if( push != NULL ){
		tmp = head->nexttoken;
		head->nexttoken = push;
		push->nexttoken = tmp;
		tmp = NULL;
	}
}

//insert to last index
void inserttoken(struct token *head, struct token *push){
	struct token *next = head;
	
	while( next->nexttoken != NULL ){
		next = next->nexttoken;
	}
	next->nexttoken = push;
	push->nexttoken = NULL;
}

// 후위 표기법으로 바꾸기 위해 연산자 우선순위에 따라 스택에 넣고 빼고를 반복해서 후위표기로 변경 
void makepostfix(struct token *head, struct token *phead, struct token *opstack){
	struct token *tmp = NULL;
	struct token *op = NULL;
	int cnt = 0;

	while( (tmp = poptoken(head)) != NULL ){
		priority(tmp);
		if( strcmp(tmp->str, ":") == 0 ){
			if( opstack->nexttoken != NULL ){
				while( (op = poptoken(opstack)) != NULL ){
					inserttoken(phead, op);
				}
			}
			inserttoken(phead, tmp);
		}
		else if( cnt != 0 ){
			if( isop(tmp->str[0]) <= 0 ){
				inserttoken(phead, tmp);
			}
			else if( isop(tmp->str[0]) == 6 ){
				cnt++;
				if( tmp->operands != 1 )
					pushtoken(opstack, tmp);
				else
					inserttoken(phead, tmp);
			}
			else if( isop(tmp->str[0]) == 7 ){
				cnt --;
				while( (op = poptoken(opstack)) != NULL ){
					if( isop(op->str[0]) == 6 ){
						break;
					}
					else{
						inserttoken(phead, op);
					}
				}
			}
			else{
				pushtoken(opstack, tmp);
			}
		}
		// token is not operater
		else if( isop(tmp->str[0]) <= 0 ){
			inserttoken(phead, tmp);
		}
		// token is '('
		else if( isop(tmp->str[0]) == 6 ){
			if( tmp->operands != 1){
				pushtoken(opstack, tmp);
				cnt++;
			}
			else
				inserttoken(phead, tmp);
		}
		// token is ')'
		else if( isop(tmp->str[0]) == 7 ){
			while( (op = poptoken(opstack)) != NULL ){
				if( isop(op->str[0]) == 6 ){
					cnt--;
					break;
				}
				else{
					inserttoken(phead, op);
				}
			}
			if( tmp->operands == 1 )
				inserttoken(phead, tmp);
		}
		// token is operater
		else{
			if( opstack->nexttoken == NULL ){
				pushtoken(opstack, tmp);
			}
			// operater it is in stack is bigger than operater will push stack
			else if( priority(tmp) >= priority(opstack->nexttoken) ){
				op = poptoken(opstack);
				if( op != NULL ){
					// if pop operater is not '(', insert that to phead
					if( isop(op->str[0]) != 6 ){
						inserttoken(phead, op);
					}
				}
				pushtoken(opstack, tmp);
			}
			else{
				pushtoken(opstack, tmp);
			}
		}
	}
	if( opstack->nexttoken != NULL ){
		while( (op = poptoken(opstack)) != NULL ){
			inserttoken(phead, op);
		}
	}
}

// 후위 표기법으로 만든 토큰 리스트를 정렬하여 중위 표기법으로 변경
struct token *sorttoken(struct token *head){
	struct token *next;
	struct token *tmp;
	struct token *tmp2;
	struct token *pre;
	struct token *preop;

		pre = head;
		next = head->nexttoken;
		if( next == NULL || next->nexttoken == NULL ){
			return head;
		}
		tmp = next->nexttoken;
		if( tmp->operands == PRE_OP || tmp->operands == POST_OP ){
			if( isopstr(next->str) != 1 ){
				tmp2 = next;
				if( tmp->operands == PRE_OP ){
					strcat(tmp->str, tmp2->str);
					pre->nexttoken = tmp;
					tmp->operands = 0;
					//free(tmp2);
				}
				else{
					strcat(tmp2->str, tmp->str);
					tmp2->nexttoken = tmp->nexttoken;
					tmp2->operands = 0;
					//free(tmp);
				}
			}
		}
		while( (next->nexttoken != NULL) && ((tmp = next->nexttoken->nexttoken) != NULL) ){
			if( tmp->ch == 1 && isopstr(tmp->str) == 1){
				if( isopstr(next->str) != 1 && isopstr(next->nexttoken->str) != 1){
					if( strcmp(next->str, next->nexttoken->str) > 0 ){
						tmp2 = next;
						pre->nexttoken = next->nexttoken;
						pre->nexttoken->nexttoken = tmp;
						next->nexttoken = tmp->nexttoken;
						tmp->nexttoken = next;
						if( strcmp(tmp->str, "<") == 0 )
							strcpy(tmp->str, ">");
						else if( strcmp(tmp->str, ">") == 0 )
							strcpy(tmp->str, "<");
						else if( strcmp(tmp->str, "<=") == 0 )
							strcpy(tmp->str, ">=");
						else if( strcmp(tmp->str, ">=") == 0 )
							strcpy(tmp->str, "<=");
						struct token *h = (struct token *) calloc(1, sizeof(struct token));
						h->str[0] = '(';
						strcat(h->str, pre->nexttoken->str);
						strcat(h->str, tmp->str);
						strcat(h->str, next->str);
						strcat(h->str, ")");
						h->nexttoken = next->nexttoken;
						pre->nexttoken = h;
						//free(tmp);
						//free(next);
						//free(next->nexttoken);
						next = pre;
					}
					else{
						struct token *h = (struct token *) calloc(1, sizeof(struct token));
						h->str[0] = '(';
						strcat(h->str, next->str);
						strcat(h->str, tmp->str);
						strcat(h->str, next->nexttoken->str);
						strcat(h->str, ")");
						h->nexttoken = tmp->nexttoken;
						pre->nexttoken = h;
						//free(tmp);
						//free(next);
						//free(next->nexttoken);
						next = pre;
					}
				}
			}
			
			else if( tmp->operands == PRE_OP || tmp->operands == POST_OP ){
				if( isopstr(next->nexttoken->str) != 1 ){
					tmp2 = next->nexttoken;
					if( tmp->operands == PRE_OP && checktype(tmp2->str) != 1){
						strcat(tmp->str, tmp2->str);
						next->nexttoken = tmp;
						tmp->operands = 0;
						//free(tmp2);
					}
					else{
						strcat(tmp2->str, tmp->str);
						tmp2->nexttoken = tmp->nexttoken;
						tmp2->operands = 0;
						//free(tmp);
					}
					continue;
				}
			}
			
			else{
				if( tmp->ch == 0 && isopstr(tmp->str) == 1){
					if( isopstr(next->str) != 1 && isopstr(next->nexttoken->str) != 1){
						if( strcmp(next->str, next->nexttoken->str) > 0 ){
							tmp2 = next;
							pre->nexttoken = next->nexttoken;
							pre->nexttoken->nexttoken = tmp;
							next->nexttoken = tmp->nexttoken;
							tmp->nexttoken = next;
							struct token *h = (struct token *) calloc(1, sizeof(struct token));
							h->str[0] = '(';
							strcat(h->str, pre->nexttoken->str);
							strcat(h->str, tmp->str);
							strcat(h->str, next->str);
							strcat(h->str, ")");
							h->nexttoken = next->nexttoken;
							pre->nexttoken = h;
							//free(tmp);
							//free(next);
							//free(next->nexttoken);
							next = pre;
						}
						else{
							struct token *h = (struct token *) calloc(1, sizeof(struct token));
							h->str[0] = '(';
							strcat(h->str, next->str);
							strcat(h->str, tmp->str);
							strcat(h->str, next->nexttoken->str);
							strcat(h->str, ")");
							h->nexttoken = tmp->nexttoken;
							pre->nexttoken = h;
							//free(tmp);
							//free(next);
							//free(next->nexttoken);
							next = pre;
						}
					}
				}
			}
			
			pre = next;
			next = next->nexttoken;
		}
	
	
	return head;
}

// 캐스팅할 때 괄호를 남기기 위한 함수
int checktype(const char *str){
	int ret = 0;
	if( strcmp(str, "char") == 0 )
		ret = 1;
	else if( strcmp(str, "int") == 0 )
		ret = 1;
	else if( strcmp(str, "long") == 0 )
		ret = 1;
	else if( strcmp(str, "float") == 0 )
		ret = 1;
	else if( strcmp(str, "double") == 0 )
		ret = 1;
	else if( strcmp(str, "struct") == 0 )
		ret = 1;
	return ret;
}
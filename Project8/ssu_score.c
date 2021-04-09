#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "ssu_file.h"
#include "ssu_student.h"
#include "ssu_string.h"
#include "ssu_opt.h"

/*
	return : 해당 빈칸 문제 정답과 학생 답안을 tokencmp()에 따라 점수를 계산하여 리턴
	argument : 정답 파일 리스트, 학생 리스트, 해당 문제의 점수
*/
float tscoring(struct token *anshead, struct token *stdhead, const float score){
	int check = A_WRONG;
	// tokencmp() 똑같은 토큰 배열 일때 0 리턴
	if( tokencmp(anshead, stdhead) == 0 ){
		check = A_RIGHT;
	}

	return score * (float)check;
}

/*
	return : 해당 프로그램 문제의 정답과 학생 답안을 점수를 계산
	argument : 프로그램 문제와 답안 파일 구조체
*/
float cscoring(struct ssu_file *prog, struct ssu_file *ans){
	char pwd[PATH_SIZE];
	char fname[PATH_SIZE];
	char *compileout;
	char *execout;
	int check = A_WRONG;
	int warncnt = 0;
	int len;
	float score;

	getcwd(pwd, PATH_SIZE);
	chdir(prog->path);
	strcpy(prog->f_name, ans->f_name);

	//compile .c file
	if( (compileout = compileprog(ans->f_name)) != NULL ){ 
		//execute program
		if( (execout = execprog(ans->f_name)) != NULL ){
			if( strcmp(execout, "TIMEOVER") == 0 ){
				score = C_TIMEOVER_SCORE;
			}
			else{
				//read output file
				if( readsfile(execout, prog) >= 0 ){
					if( ans->text == 0 ){
						return A_WRONG;
					}
					if( stringcmp(ans->text, prog->text) == 0 ){
						check = A_RIGHT;
						if( (warncnt = warncount(compileout)) >= 0 ){	
							score = (check * ans->score) - (warncnt * C_WARNING_SCORE);
						}
					}
				}
			}
		}
		else{
			memset(buf, 0, BUF_SIZE);
			sprintf(buf, "rm -rf %s", execout);
			system(buf);
		}
	}
	// 컴파일 에러 시
	else{
		score =  C_ERROR_SCORE;
	}
	// e 옵션이 존재하지 않을 때 컴파일 오류 정보 삭제
	if( prog == ans || (flag & E_FLAG) != E_FLAG ){
		len = strlen(ans->f_name);
		strcpy(fname, ans->f_name);
		fname[len-2] = 0;
		memset(buf, 0, BUF_SIZE);
		sprintf(buf, "rm -rf %s_error.txt", fname);
		system(buf);
	}

	chdir(pwd);

	return score;
}

/*
	return : 컴파일 후에 나오는 에러 파일 이름 리턴
	argument : 컴파일 할 파일 이름
*/
char *compileprog(const char *filename){
	char *fname;
	char cmd[128];
	char *ret = NULL;
	int length;
	int i;
	char *topt="";

	fname = (char *) calloc(PATH_SIZE, sizeof(char)); 
	memset(buf, 0, BUF_SIZE);
	length = strlen(filename);
	strncpy(buf, filename, length-2);
	buf[length-2] = 0;

	if( (flag & T_FLAG) == T_FLAG ){
		for(i=0; i<5; i++){
			if( strcmp(t_arg[i], buf) == 0 ){
				topt = "-lpthread";
			}
		}
	}
	// system()을 이용하기 위해 컴파일 명령어를 저장
	sprintf(cmd, "gcc -o %s.stdexe %s %s > %s_error.txt 2>&1", buf, filename, topt, buf);
	//컴파일 수행
	system(cmd);
	strcat(buf, ".stdexe");

	usleep(100);
	// .stdexe파일이 존재하면 컴파일 성공
	if( access(buf, F_OK) == 0 ){
		memset(buf, 0, BUF_SIZE);
		strncpy(buf, filename, length-2);
		buf[length-2] = 0;
		strcat(buf, "_error.txt");
		strcpy(fname, buf);
		ret = fname;
	}
	return ret;
}

/*
	return : 실행 후 생성되는 파일 이름 리턴
	argument : 실행할 파일 이름을 인자로 받음
*/
char *execprog(const char *filename){
	char *fname;
	char cmd[128];
	char killcmd[128];
	char pscmd[128];
	char rmcmd[128];
	char buf2[BUF_SIZE];
	char *ret = NULL;
	int length;
	int filedes;
	time_t first_t, second_t;

	fname = (char *) calloc(PATH_SIZE, sizeof(char)); 
	memset(buf, 0, BUF_SIZE);
	length = strlen(filename);
	strncpy(buf, filename, length-2);
	buf[length-2] = 0;
	// 컴파일과 수행 작업 명령어
	sprintf(cmd, "./%s.stdexe > %s.stdout 2>&1 &", buf, buf);
	sprintf(killcmd, "killall -9 %s.stdexe > %s 2>&1 & ", buf, buf);
	sprintf(pscmd, "ps -ef | grep %s.stdexe > %s 2>&1 &", buf, buf);
	sprintf(rmcmd, "rm -rf %s", buf);
	
	system(cmd);
	first_t = time(NULL);
	// ps 명령어를 system()를 통해 프로세스가 실행중인지 확인
	while(1){
		system(pscmd);
		second_t = time(NULL);
		sleep(1);
		// ps 명령어로 생성되는 파일에서 실행되고 있다면 "./" 스트링이 존재
		if( access(buf, F_OK) == 0 ){
			if( (filedes = open(buf, O_RDONLY)) > 0 ){
				if( read(filedes, buf2, BUF_SIZE) >= 0 ){
					if( strstr(buf2, "./") == NULL ){
						strcpy(fname, buf);
						strcat(buf, ".stdout");
						ret = buf;
						close(filedes);
						break;
					}
				}

			}
			// 시간 초과 시 TIMEOVER 스트링 리턴
			if( (difftime(second_t, first_t) - 5.0) >= 0 ){
				system(killcmd);
				ret = "TIMEOVER";
				close(filedes);
				break;
			}
			close(filedes);
		}
	}
	// ps 명령어시 나오는 파일 제거
	if( access(fname, F_OK ) == 0 ){
		system(rmcmd);
	}

	return ret;
}

/*
	return : void
	argument : 전체 학생 디렉토리 path
*/
void scoring(const char *stdpath){
	struct ssu_student *std = students->next;
	char pwd[PATH_SIZE];
	char errpath[PATH_SIZE];
	float score = 0;
	float avg = 0;
	int cnt = 0;

	getcwd(pwd, PATH_SIZE);
	anscompile();

	if( chdir(stdpath) != 0 ){
		fprintf(stderr, "STD_DIR <%s> doesn't exist\n", stdpath); 
	}
	while( std != NULL ){
		cnt ++;
		// 해당 학생을 전체 파일을 점수내는 함수를 통해 학생의 총점과 전체 평균 계산
		if( (score = stdscoring(std->s_num)) >= 0 ){
			std->totalscore = score;
			avg += score;
		}
		// p 옵션이 있으면 해당학생 종료 후에 점수 출력
		if( (flag & P_FLAG) == P_FLAG ){
			printf("%s is finished.. : %.2f\n", std->s_num, std->totalscore);
		}
		else{
			printf("%s is finished..\n", std->s_num);
		}
		// e 옵션이 있으면 _error.txt파일 이동
		if( (flag & E_FLAG) == E_FLAG ){
			chdir(pwd);
			if( access(e_arg, F_OK) != 0 ){
				mkdir(e_arg, 0777);
			}
			chdir(e_arg);
			getcwd(errpath, PATH_SIZE);
			mkdir(std->s_num, 0777);
			chdir(std->path);
			memset(buf, 0, BUF_SIZE);
			sprintf(buf, "mv *_error.txt %s/%s/", errpath, std->s_num);
			system(buf);
			chdir(pwd);
			chdir(stdpath);
		}
		std = std->next;
		score = 0;
	}
	// 전체 평균 출력
	if( (flag & P_FLAG) == P_FLAG ){
		avg /= cnt;
		printf("Total average : %.2f\n", avg);
	}
	chdir(pwd);
}

/*
	return : 학생의 전체 점수를 리턴
	argument : 점수를 내고 싶은 학생의 학번
*/
float stdscoring(const char *stdnum){
	struct ssu_student *std;
	struct ssu_file *ansfp = ansfile->next;
	char pwd[PATH_SIZE];
	float total = 0;
	float score = 0;
	int i = 0;
	static int cnt = 0;

	getcwd(pwd, PATH_SIZE);

	if( (std = findstd(stdnum)) == NULL ){
		return -2;
	}
	if( chdir(std->path) != 0 ){
		fprintf(stderr, "<%s> doesn't exist\n", stdnum);
		return -1;
	}
	cnt++;
	while( ansfp != NULL ){
		if(	(ansfp->f_name)[0] == '\0' ){
			continue;
		}
		//get student file that is same name
		readsfile(ansfp->f_name, &std->buf_file);

		//if ansfp is text file
		if( ansfp->f_num == T_FILE){
			if( cnt == 1 ){
				ansfp->head = str2token(ansfp->text, ansfp->head);
			}
			std->headtoken = str2token(std->buf_file.text, std->headtoken);
			if( (score = tscoring(ansfp->head, std->headtoken, ansfp->score)) >= 0 ){
				total += score;
				std->score[i] = score;
			}

		}
		//if ansfp is .c file
		else if( ansfp->f_num == C_FILE){
			if( (score = cscoring(&std->buf_file, ansfp)) >= 0 ){
				total += score;
				std->score[i] = score;
			}
		}
		i++;
		ansfp = ansfp->next;
	}
	if( chdir(pwd) != 0 ){
		fprintf(stderr, "<%s> doesn't exist\n", stdnum);
		return -1;
	}
	return total;
}

// 정답 파일은 한번에 모든 것을 컴파일
void anscompile(){
	struct ssu_file *ansfp = ansfile->next;

	while( ansfp != NULL ){
		if( ansfp->f_num == C_FILE ){
			cscoring(ansfp, ansfp);
		}
		ansfp = ansfp->next;
	}
}

// warning이 있을 경우 감점을 위해 계산하여 warning 개수 리턴 인자로 학생 path 컴파일 후 생성 파일
int warncount(const char *pathname){
	char *word;
	int filedes;
	int length;
	int cnt = 0;
	// 컴파일 후에 생기는 파일 오픈
	if( (filedes = open(pathname, O_RDONLY)) < 0 ){
		fprintf(stderr, "open error for %s\n", pathname);
		return -1;
	}
	memset(buf, 0, BUF_SIZE);
	// 끝까지 읽으면서 warning의 수를 체크
	while( (length = read(filedes, buf, BUF_SIZE)) > 0 ){
		word = buf;
		while( (word = strstr(word, "warning:")) != NULL ){
			cnt++;
			word += 7;
		}
	}
	return cnt;
}



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include "ssu_student.h"
#include "ssu_file.h"
#include "ssu_string.h"
#include "ssu_score.h"
#include "ssu_opt.h"

void myflush();
int getoption(int, char*[]);

int flag;
char e_arg[PATH_SIZE];
char c_arg[5][S_NUM];
char t_arg[5][FNAME_SIZE];
char ansdir[PATH_SIZE];
char stddir[PATH_SIZE];

int main(int argc, char *argv[]){
	FILE *fp;
	char pwd[PATH_SIZE];
	int e_no;
	int type=0;
	int i;
	int check = 1;
	char *s_table = "score_table.csv";
	char *s_score = "score.csv";

	if( argc < 2 ){
		fprintf(stderr, "usage : %s <STD_DIR> <ANS_DIR> [OPTION]\n", argv[0]);
		exit(1);
	}
	getcwd(pwd, PATH_SIZE);
	if( getoption(argc, argv) != 0 ){
		// 인자로 받은 폴더를 존재 여부 확인
		if( access(ansdir, F_OK) != 0 ){
			fprintf(stderr, "%s doesn't exist\n", ansdir);
			check = 0;
		}
		if( access(stddir, F_OK) != 0 ){
			fprintf(stderr, "%s doesn't exist\n", stddir);
			check = 0;
		}
		// 해당 폴더들이 존재하면 채점 진행
		if( check != 0){
			/*
				여러가지 오류로 인한 해당 디렉토리로 이동이 불가는 할 경우 출력
				getans는 정답을 linked list에 저장하는 함수
				getallstd는 학생들을 linked list로 저장
			*/
			if( (e_no = getans(ansdir)) < 0 ){
				fprintf(stderr, "Can't access %s\n", ansdir);
				exit(1);
			}
			if( (e_no = getallstd(stddir)) < 0 ){
				fprintf(stderr, "Can't access %s\n", stddir);
				exit(1);
			}
			// 정답폴더에 있는 score_table.csv을 읽음
			chdir(ansdir);
			if( (fp = fopen(s_table, "rt")) == NULL){
				printf("%s file doesn't exist in %s\n", s_table, ansdir);
				// 존재하지 않으면 새로 score_table.csv 작성
				fp = fopen(s_table, "wt");
				printf("1. input blank question and program question's score. ex) 0.5 1\n");
				printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
				// type 입력
				do{
					printf("select type >> ");
					scanf("%d", &type);
					myflush();
				}while( type != 1 && type != 2 );
				savestable(fp, type);
			}
			// score table을 로드
			else{
				loadstable(fp);
			}
			fclose(fp);
			chdir(pwd);
			// 채점 시작
			printf("grading student's test papers..\n");
			scoring(stddir);
			// 채점이 종료 되고 score.csv 저장
			if( (fp = fopen(s_score, "wt")) != NULL ){
				savestdscore(fp);
			}
		}
	}
	// c옵션이 있으면 score.csv에서 인자로 받은 학생의 점수 출력
	if( (flag & C_FLAG) == C_FLAG ){
		if( (fp = fopen(s_score, "rt")) == NULL ){
			fprintf(stderr, "score.csv doesn't exist\n");
		}
		else{
			memset(buf, 0, BUF_SIZE);
			while( fscanf(fp, "%s", buf) != EOF ){
				for(i=0; i<5; i++){
					if( strcmp(c_arg[i], "") == 0 ){
						continue;
					}
					// c_arg에 저장한 학생 아이디에 맞으면 출력
					if( strstr(buf, c_arg[i]) != NULL ){
						printf("%s's score : ", c_arg[i]);
						memset(buf, 0, BUF_SIZE);
						while( fscanf(fp, "%s", buf) != EOF ){
							if( strchr(buf, ',') == NULL ){
								printf("%s\n", buf);
								break;
							}
							memset(buf, 0, BUF_SIZE);
						}
					}
				}
				memset(buf, 0, BUF_SIZE);
			}
		}
	}
	// h 옵션이 있을 경우 출력
	if( (flag & H_FLAG) == H_FLAG){
		printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
		printf("Option :\n");
		printf(" -e <DIRNAME>   print error on 'DIRNAME/ID/qname_error.txt' file\n");
		printf(" -t <QNAMES>    compile QNAME.C with -lpthread option\n");
		printf(" -h             print usage\n");
		printf(" -p             print student's score and total average\n");
		printf(" -c <IDS>       print ID's score\n");
	}
	exit(0);
}
/*
	입력버퍼를 비워주는 함수
*/
void myflush(){
	char c;
	while( (c = getchar()) != '\n' );
}

/*
	return : 채점을 진행하지 않아도 되는 옵션은 1을 리턴 그 외 0을 리턴
	argument: 메인에서 받은 int argc, char *argv[] 
	가변인자 옵션을 처리하기 위한 함수
*/
int getoption(int argc, char *argv[]){
	char option;
	int i=0;
	int cnt;
	int ret = 0;
	// c, h 옵션이 있다면 1을 리턴
	if( (strcmp(argv[1], "-c") != 0) && (strcmp(argv[1], "-h") != 0) ){
		strcpy(stddir, argv[1]);
		strcpy(ansdir, argv[2]);
		ret = 1;
	}
	// 가변인자 처리
	while( i < argc ){
		// c옵션일 경우 5개까지 받아서 c_arg배열에 넣어줌
		if( strcmp(argv[i], "-c") == 0 ){
			cnt = 0;
			i++;
			while( i < argc ){
				if( strchr(argv[i], '-' ) != NULL ){
					break;
				}
				// 5개 초과는 출력
				if( cnt >= 5 ){
					printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
				}
				else{
					strcpy(c_arg[cnt], argv[i]);
					cnt++;
				}
				i++;
			}
			// 옵션 c의 인자가 없을 경우
			if( cnt == 0 ){
				printf("Option c : -c <IDS>			print ID's score\n");
			}
		}
		// 가변인자를 받는 t옵션 처리
		else if( strcmp(argv[i], "-t") == 0 ){
			cnt = 0;
			i++;
			while( i < argc ){
				if( strchr(argv[i], '-' ) != NULL ){
					break;
				}
				// 5개 초과는 출력
				if( cnt >= 5 ){
					printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
				}
				// 5개 이하는 저장
				else{
					strcpy(t_arg[cnt], argv[i]);
					cnt++;
				}
				i++;
			}
			// 옵션 t의 인자가 없을 경우에 출력
			if( cnt == 0 ){
				printf("Option t : -t <QNAMES>    compile QNAME.C with -lpthread option\n");
			}

		}
		else{
			i++;
		}
	}

	//get option 플래그에 옵션이 있는 경우 체크
	while( (option = getopt(argc, argv, "e:pthc")) > 0 ){
		switch(option){
			case 'e':
				flag |= E_FLAG;
				strcpy(e_arg, optarg);
				break;
			case 'p':
				flag |= P_FLAG;
				break;
			case 't':
				flag |= T_FLAG;
				break;
			case 'h':
				flag |= H_FLAG;
				break;
			case 'c':
				flag |= C_FLAG;
				break;
		}
	}
	return ret;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ssu_file.h"
#include "ssu_string.h"
#include "ssu_student.h"

// 정답 파일들을 linked list로 넣어 놓음
struct ssu_file *ansfile;

/*
	return : 정답파일을 성공적으로 저장시 0 리턴, 디렉토리 오픈에러 -1, 폴더 이동에러 -2 리턴
	argument : 정답 파일의 path
*/
int getans(const char *anspath){
	DIR *ansdir = NULL;
	struct dirent *dentry;
	struct stat statbuf;
	struct ssu_file *ans;
	struct ssu_file *tmpans;
	char pwd[PATH_SIZE];
	// 이동하기 전 기존 디렉토리 저장
	getcwd(pwd, PATH_SIZE);

	// Need answer path
	if( anspath == NULL ){
		return -1;
	}

	if( (ansdir = opendir(anspath)) == NULL ){
		return -1;
	}

	if( chdir(anspath) != 0 ){
		return -2;
	}
	// 정답 linked list 헤더 생성
	if( ansfile == NULL ){
		ansfile = (struct ssu_file*) calloc(1,sizeof(struct ssu_file));
	}
	// 폴더를 이동해서 파일일 경우 저장, 디렉토리일 경우 해당 디렉토리로 이동
	while( (dentry = readdir(ansdir)) != NULL ){
		if( dentry->d_name[0] == '.'){
			continue;
		}
		// dentry 정보 가져옴
		stat(dentry->d_name, &statbuf);
		// 디렉토리이면 해당 디렉토리로 이동
		if( (statbuf.st_mode & S_IFMT) == S_IFDIR ){
			getans(dentry->d_name);
			continue;
		}
		// 파일일 경우 .txt .c가 이름에 있는 파일을 가져옴
		else if( (statbuf.st_mode & S_IFMT) == S_IFREG ){
			// 오류 예외 처리 
			if( strstr(dentry->d_name, ".csv") != NULL || strstr(dentry->d_name, "_error.txt") != NULL ){
				continue;
			}
			if( strstr(dentry->d_name, ".txt") != NULL || strstr(dentry->d_name, ".c") != NULL ){
				ans = (struct ssu_file*) calloc(1, sizeof(struct ssu_file));
				// 해당 파일을 읽는 함수를 통한 파일에 내용, 문제 번호 등 정보를 가져옴
				readsfile(dentry->d_name, ans);
				tmpans = getprevans(ans);
				ans->next = tmpans->next;
				tmpans->next = ans;
			}
		}

	}
	closedir(ansdir);
	// 원래 디렉토리로 이동
	if( chdir(pwd) != 0 ){
		return -2;
	}

	return 0;
}

/*
	return : 파일을 성공적으로 읽은 경우 0 리턴 file open 에러시 -3, read 에러시 -4 리턴
	argument : 파일이름 (fname), 해당 파일을 연결할 전 ssu_file 구조체 포인터
*/
int readsfile(const char *fname, struct ssu_file *s_file){
	char *splitp;
	char buf2[10];
	int filedes;
	int length;
	int len;
	char pwd[PATH_SIZE];
	//파일 오픈 에러
	if( (filedes = open(fname, O_RDONLY)) < 0 ){
		return -3;
	}
	
	memset(buf, 0, BUF_SIZE * sizeof(char));
	// 파일 read 에러, 정답 및 답안 파일의 내용이 1024 바이트를 넘지 않는다는 가정
	if( (length = read(filedes, buf, BUF_SIZE)) < 0 ){
		return -4;
	}
	if( length >= 0 ){
		if(s_file->f_num == 0){
			buf[length] = 0;
			// 파일 이름에 -가 있을 경우 문제번호-서브문제번호 로 저장
			if( (splitp = strchr(fname, '-')) != NULL ){
				len = (splitp - fname) / sizeof(char);	
				strncpy(buf2, fname, len);
				buf2[len] = 0;
				s_file->p_num = atoi(buf2);
				strcpy(buf2, splitp+1);
				len = strlen(splitp+1);
				// .txt 파일이면 끝에 .txt를 지우고 서브문제번호를 저장
				// 아니면 .c파일밖에 없다는 가정으로 끝에 .c 지우고 서브문제번호 저장
				if( strstr(fname, ".txt") != NULL ){
					buf2[len-4] = 0;
				}
				else{
					buf2[len-2] = 0;
				}
				s_file->sub_num = atoi(buf2);
			}
			// 파일이름에 -가 없으면 문제번호 확장자 지우고 바로 저장
			else{
				len = strlen(fname);
				strcpy(buf2, fname);
				if( strstr(fname, ".txt") != NULL ){
					buf[len-4] = 0;
				}
				else{
					buf2[len-2] = 0;
				}
				s_file->p_num = atoi(buf2);
				s_file->sub_num = 999;
			}

			strcpy(s_file->f_name, fname);
			s_file->f_num = 0;
			getcwd(pwd,PATH_SIZE);
			strcpy(s_file->path, pwd);
		}
		// c 파일이면 파일 넘버 저장
		if( strstr(fname, ".c") != NULL ){
			s_file->f_num = C_FILE;
		}
		else{
			if( s_file->text != NULL ){
				free(s_file->text);
			}
			// 파일 내용 저장
			s_file->text = (char*) calloc(length+1, sizeof(char));
			strcpy(s_file->text, buf);
		}
	}
	close(filedes);
	return 0;
}

/*
	return : 인자로 들어온 ssu_file 구조체보다 이전에 있는 ssu_file구조체 포인터 리턴
	argument : linked list에 넣을 ssu_file 구조체 포인터
	linked list에 넣을 때 바로 정렬해서 넣기 때문에 ssu_file 의 문제 번호에 따라 그 전에 있는
	ssu_file을 찾는 함수
*/
struct ssu_file *getprevans(struct ssu_file *ans){
	struct ssu_file *ansnext=ansfile;
	
	while( ansnext->next != NULL ){
		// 넣으려는 구조체가 문제 번호가 크면 다음으로 넘어감
		if( ansnext->next->p_num < ans->p_num ){
			ansnext = ansnext->next;
		}
		// 문제 번호가 같다면 서브 문제 번호를 비교
		else if( ansnext->next->p_num == ans->p_num ){
			if( ansnext->next->sub_num < ans->sub_num){
				ansnext = ansnext->next;
			}
			else{
				break;
			}
		}
		// 문제 번호가 작다면 넣을 위치를 찾음
		else if( ansnext->next->p_num > ans->p_num ){
			break;
		}
	}

	return ansnext;
}
/*
	return : linked list에서 문제 번호를 찾아서 해당 ssu_file *를 리턴
	argument : linked list에서 찾을 문제 번호를 가진 이름을 인자로 넣어줌 ex) 1-1, 1-2, 10
*/
struct ssu_file *findans(const char *ansnum){
	struct ssu_file *ansnext = ansfile;
	// 해당 이름이 똑같으면 break;
	while( ansnext->next != NULL ){
		if( strcmp(ansnext->next->f_name, ansnum) == 0 ){
			break;
		}
		else{
			ansnext = ansnext->next;
		}
	}

	return ansnext->next;
}

/*
	return : void
	argument : score_table.csv 파일을 저장하기 위한 해당 FILE *, 저장할 때 type에 따른 저장
		type 1 - 모든 빈칸 문제 동일 점수, 모든 프로그램 문제 동일 점수
		type 2 - 문제에 대한 점수 각각 입력
*/
void savestable(FILE *s_table, int type){
	struct ssu_file *ansnext = ansfile;
	float blkscore, progscore;
	// type 1 이면 한번에 저장
	if( type == 1 ){
		printf("Input value of blank question : ");
		scanf("%f", &blkscore); 
		printf("Input value of program question : ");
		scanf("%f", &progscore); 
	}
	while( (ansnext = ansnext->next) != NULL ){
		if( type == 1 ){
			if( ansnext->f_num == T_FILE ){
				ansnext->score = blkscore;
			}
			else if( ansnext->f_num == C_FILE ){
				ansnext->score = progscore;
			}
		}
		// type 2 이면 각각 입력 받아서 저장
		else if( type == 2 ){
			printf("Input of %s: ", ansnext->f_name);
			scanf("%f", &blkscore);
			ansnext->score = blkscore;
		}
		fprintf(s_table, "%s, %.2f\n", ansnext->f_name, ansnext->score);
	}
}

/*
	return : void
	argument : score_table.csv를 읽을 FILE *
*/			
void loadstable(FILE *s_table){
	struct ssu_file *ans;
	float score;
	int length;

	memset(buf, 0, BUF_SIZE);
	// 형식에 맞게 저장한 파일을 형식에 맞게 읽어서 정답 linked list에 해당 문제와 점수를 저장
	while( fscanf(s_table, "%s %f\n", buf, &score) > 0 ){
		length = strlen(buf);
		buf[length-1] = 0;
		ans = findans(buf);
		strncpy(ans->f_name, buf, length-1);
		ans->score = score;
	}
}

/*
	return : void
	argument : 저장할 파일의 FILE * (score.csv)
*/
void savestdscore(FILE *s_score){
	struct ssu_file *ans = ansfile;
	struct ssu_student *std = students;
	int i=0;

	// 모든 학생 linked list 끝까지 반복
	while( std != NULL ){
		i=0;
		// 모든 정답 linked list 끝까지 반복
		while( ans != NULL ){
			// 학생 linked list 처음이라면 공백
			if( std == students ){
				fprintf(s_score, "%s, ", ans->f_name);
			}
			// 정답 linked list 처음이라면 학생 번호 저장
			else if( ans == ansfile ){
				fprintf(s_score, "%s, ", std->s_num);
			}
			// 나머지 해당 학생의 해당 문제 점수 저장
			else{
				fprintf(s_score, "%.2f, ", std->score[i]);
				i++;
			}
			ans = ans->next;
		}
		// 마지막이면 해당 학생의 총합 저장
		if( std == students ){
			fprintf(s_score, "%s\n", "sum");
		}
		else{
			fprintf(s_score, "%.2f\n", std->totalscore);
		}
		std = std->next;
		ans = ansfile;
	}
}

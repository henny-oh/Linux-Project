#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ssu_student.h"
#include "ssu_string.h"

struct ssu_student *students;
/*
	return : 모든 학생을 다 읽으면 0리턴
	argument : 학생디렉토리들이 있는 디렉토리 path
*/
int getallstd(const char* stdpath){
	DIR *stddir;
	struct dirent *dentry;
	struct stat statbuf;
	struct ssu_student *std = NULL;
	struct ssu_student *tmpstd = NULL;
	char pwd[PATH_SIZE];

	getcwd(pwd, PATH_SIZE);
	
	if( (stddir = opendir(stdpath)) == NULL ){
		return -1;
	}

	if( chdir(stdpath) != 0 ){
		return -2;
	}

	if( students == NULL ){
		students = (struct ssu_student*)calloc(1, sizeof(struct ssu_student));
	}
	// 학생들의 디렉토리를 학생이름으로 저장
	while( (dentry = readdir(stddir)) != NULL ){
		if( dentry->d_name[0] == '.' ){
			continue;
		}
		stat(dentry->d_name, &statbuf);
		if( (statbuf.st_mode & S_IFMT) == S_IFDIR ){
			chdir(dentry->d_name);
			std = (struct ssu_student*)calloc(1, sizeof(struct ssu_student));
			strcpy(std->s_num, dentry->d_name);
			getcwd(std->path, PATH_SIZE);
			tmpstd = getprevstd(std);
			std->next = tmpstd->next;
			tmpstd->next = std;
			chdir("../");
		}
	}
	if( chdir(pwd) != 0 ){
		return -2;
	}
	return 0;
}

/*
	return : ssu_student *로 자신보다 전의 학생 구조체 포인터 리턴
	argument : ssu_student *로 정렬 시에 인자로 넣은 학생보다 앞선번호의 구조체 리턴
*/
struct ssu_student *getprevstd(struct ssu_student *std){
	struct ssu_student *stdnext=students;
	
	while( stdnext->next != NULL ){
		// strcmp를 이용하여 학생번호 비교
		if( strcmp(stdnext->next->s_num, std->s_num) < 0 ){
			stdnext = stdnext->next;
		}
		else{
			break;
		}
	}

	return stdnext;
}
/*
	return : 인자로 받은 학생번호 스트링과 같은 학생의 구조체 리턴
	argument : 찾을 학생 번호
*/
struct ssu_student *findstd(const char *stdnum){
	struct ssu_student *stdnext = students;

	while( stdnext->next != NULL ){
		if( strcmp(stdnext->next->s_num, stdnum) == 0 ){
			break;
		}
		else{
			stdnext = stdnext->next;
		}
	}

	return stdnext->next;
}

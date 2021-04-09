### Linux-Project

*****

## project1
-리눅스의 내장 명령어를 실행할 수 있는 쉘의 기능을 하는 “SSUShell”, ps 명령어와 동일한 기능의 “pps”, top 명령어 기능을 하는 “ttop”를 구현한다. “SSUShell”에서 “pps”/ “ttop”를 입력하면 해당하는 기능들이 실행되어야 한다. 
“SSUShell”은 직접 명령어를 입력하는 대화식 모드와 실행할 명령어들이 포함된 파일을 읽은 배치식 모드가 가능하다. 배치식모드는 한 줄씩 명령어를 읽어 차례로 수행하고, 대화식 모드는 일반 쉘과 같이 입력된 명령어를 수행한다. 입력을 띄어쓰기를 기준으로  토큰을 나누고 이를 인자로 명령어를 실행한다. Fork하여 자식프로세스를 생성하고 자식 프로세스에서 exec함수를 호출하여 실행한다. 부모프로세스에서는 자식 프로세스가 끝나기 전까지 멈추며 자식프로세스가 정상적으로 종료되었는지 확인한다. 파이프 명령어도 수행할 수 있으며 일반 쉘과 동일하게 출력된다. 
“pps”는 “ps”를 구현한 것이다. “ps”명령어는 돌아가는 프로세스를 확인할 수 있는 명령어이다. 옵션(a-터미널과 연결된 프로세스, u-프로세스 소유자를 기준으로 출력, x-데몬 프로그램처럼 터미널에 종속되지 않는 프로세스 출력)도 수행할 수 있고, 뿐만 아니라 복합 명령어(au-옵션 a에 해당하는 것을 BSD포맷으로 출력, ux-옵션 x에 해당하는 것을 BSD 포맷으로 출력, ax- 시스템에서 동작 중인 모든 프로세스를 출력, aux-ax옵션에 해당하는 것을 BSD 포맷으로 출력)도 실행 가능하다.

*****
## project2
-xv6에서 프로세스 정보가 어떻게 저장되고 관리되는 지 확인한다. 또한 리턴값 등 유형에 맞게 시스템 호출 함수를 추가한다. proc.c, syscall.c, syscall.h, sysproc.c, user.h, usys.S, defs.h 에 코드를 추가하여 함수를 등록하고 정의한다. 또한 기존 라운드로빈인 스케줄러를 라운드로빈 스케줄러로 변경한다. 


*****
## project3
-Lock을 사용하여 쓰레드가 동시에 데이터에 접근하지 못하는 프로그램을 구현한다. 
	숫자를 생성하는 마스터 쓰레드와 숫자를 소비하는 워커 쓰레드를 구현한다. 지정된 크기에 버퍼만큼 숫자를 생성하고 가득 차면 생성을 멈추고 워커 쓰레드가 버퍼를 차례로 소비한다. 더 이상 소비할 버퍼가 없으면 다시 마스터 쓰레드가 숫자를 생성하고, 이를 지정된 숫자만큼 생성되고 소비 될 때까지 반복한다. 
	Read/write lock을 구현한다. 첫번째 방법에서는 쓰기 쓰레드가 lock을 획득하기 위해 기다리고 있는 경우 읽기 쓰레드가 lock을 요청하면 우선으로 이전의 읽기 쓰레드와 함께 lock을 획득한다. 두번째 방법은 쓰기 쓰레드가 lock을 대기하고 있는 상태면 읽기 쓰레드는 쓰기 쓰레드가 lock을 획득하고 풀기 전 까지 대기한다.
	세마포어를 구현한다. 다운하는 함수는 쓰레드가 멈추고 해당 쓰레드는 큐에 들어간다. 업하는 함수를 사용하면 큐에 들어간 쓰레드를 차례로 깨운다. 이렇게 생성한 세마포어 구현 함수들을 사용하여 3개의 쓰레드가 10번씩 차례로 출력하는 것을 구현한다.


*****
## project4
-메모리를 동적으로 할당하고 사용하는 사용자 지정 메모리 관리자를 구현한다. Malloc과 free 함수의 작동 방식을 구현한다. Mmap을 사용하여 사용할 메모리 페이지를 할당하고 주소를 8바이트씩 사용한다. 8바이트 씩 구조체에 주소를 넣어놓고 사용여부를 구조체 안에 표시한다. 사용하고 있는 주소에 할당하는 이중 할당은 허용하지 않는다. 구조체를 확인해 할당하려는 메모리 크기만큼 사용하지 않는 빈 곳을 찾아 할당해 준다. 메모리 해제 시 munmap을 사용해 해제한다.


*****
## project5
-파일이 생성, 제거, 읽기, 쓰기가 진행될 때 일어나는 디스크를 구현한다. 첫블록은 슈퍼블록으로 다른 블럭들의 정보가 들어있는 블럭으로 아이노드와 데이터 블록의 사용여부를 나타낸다. 다음 4개의 블록은 아이노드 구조체가 들어있는 블럭으로 한 블럭에 2개의 아이노드가 들어가 있다. 아이노드에는 파일의 크기, 이름, 파일의 데이터를 보관하고 있는 데이터블럭의 포인터를 4개 저장할 수 있는 공간이 있다. 나머지 30개는 데이터 블록으로 데이터들을 실제적으로 저장하는 곳이다.


*****
## project6
-ssu_mntr을 실행하게 되면 특정 디렉토리 안의 파일이 생성, 제거, 수정 했던 내용과 시간이 log.txt 파일에 찍힌다. 프로그램이 종료되어도 이 모니터링은 백그라운드에서 실행되며 별도로 종료할때까지 계속된다. 즉, 계속 특정 디렉토리 안 파일들의 정보를 확인하고 있는 것이다.
프롬프트 창에 DELETE 명령어를 입력하면 특정 디렉토리 안의 특정 파일은 trash 폴더 안 files 폴더 안에 담기고 이 명령어를 실행한 시간, 이 파일의 mtime, 이 파일의 원래 있었던 절대 경로가 파일이름으로 trash 폴더 안 info 폴더 안에저장된다. ENDTIME이 주어지면 해당 시간에 이 명령어를 실행한다. 이 파일이 특정 디렉토리 안에 없을 시 에러 메시지 출력 후 프롬프트로 돌아간다. ioption 입력 시 파일을 trash 폴더 안으로 이동 시키지 않고 바로 삭제해준다. roption 입력 시 지정 시간에 DELETE 명령어를 수행하기 전 삭제 여부를 확인한다.
SIZE 명령어는 파일 /디렉토리 크기를 출력하는 명령어이다. doption 입력 시 입력된 깊이 단계의 하위 디렉토리 사이즈까지 출력한다. 
RECOVER 명령어는 trash 폴더 안 files 폴더 내부의 파일을 DELETE을 실행하기 전에 있던 경로로 다시 이동하는 명령어이다. 같은 이름의 파일이 여러 개 있을 경우, 삭제 시간 과 mtime을 출력하여 파일을 구분하여 보여주고 입력을 받아 복구할 파일을 정해 명령어를 실행한다. loption 입력 시 오래된 파일들부터 차례로 출력하고 명령어를 실행한다.
TREE 명령어는 특정 디렉토리 안의 파일 구조들을 TREE 형태로 보여주는 명령어이다.
EXIT은 프롬프트 프로그램을 종료하는 명령어로 모니터링은 종료되지 않는다.
HELP는 매뉴얼을 출력해주는 명령어로 지정되지 않은 문자를 입력 시에도 HELP 명령어가 수행된다.


*****
## project7
-“ssu_crontab” 프로그램을 실행하면 실행하고자 하는 시간과 명령어를 “ssu_crontab_file”에 저장하고 삭제할 수 있다. “ssu_crond”프로그램은 “ssu_crontab_file”에 등록된 명령어들을 특정 시간에 맞춰 실행하는 프로그램이다. “ssu_crontab” 프로그램에서 명령어 등록이 성공하면 “add”, 명령어를 삭제하면 “remove”, “ssu_crond” 프로그램에서 정삭적으로 명령어가 수행되면 “run”이라는 명령어를 “ssu_crontab_log”에 저장한다. 이때, 일어난 시간과 명령어의 실행 시간, 명령어를 함께 저장한다.

-ssu_crontab:주기적으로 실행하고자 하는 명령어를 등록 및 삭제하는 프로그램

 ```
1.명령어 add <실행주기> <명령어>
 ```
실행주기와 명령어를 ssu_crontab_file에 추가하는 명령어이다. 실행주기에는 분,시,일,월,요일로 구성되어 있고 각 항목은 숫자, ‘*’(해당 필드 모든 값), ‘-’(범위지정- 연결된 사이 모든 값), ‘,’(목록),‘ /’(앞에 나온 주기의 범위를 뒤에 나온 숫자 만큼 건너 뛴다.)로 설정할 수 있다. 실행주기 입력이 잘못된 경우 에러 처리 후 다시 프롬프트를 출력한다. 
EX)2-5/2->2분,4분에 실행, */2->0분,2분,4분....에 실행

 ```
2.명령어 remove <COMMAND_NUMBER>
 ```
입력한 번호의 명령어를 제거한다. 프로그램 실행하면 “ssu_crontab_file”에 저장된 명령어들을 순서와 함께 출력하는데 이 정보를 보고 제거할 명령어를 선택한다. 해당 숫자의 명령어는 “ssu_crontab_file”에서 삭제되어 진다.

 ```
3.명령어 exit
 ```
프로그램을 종료한다.

-ssu_crond:“ssu_crontab” 프로그램을 통해 등록된 명령어를 특정 시간에 맞춰 실행하는 프로그램
“ssu_crontab_file”을 주기적으로 읽어 저장된 명령어를 시간에 맞춰 실행한다. “ssu_crontab_file”의 변화를 반영해야 한다. 

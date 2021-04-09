#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "flash.h"

// 필요한 경우 헤더파일을 추가한다

FILE *flashfp;	// fdevicedriver.c에서 사용
extern int dd_write(int ppn, char *pagebuf);
extern int dd_read(int ppn, char *pagebuf);
extern int dd_erase(int pbn);
// 이 함수는 FTL의 역할 중 일부분을 수행하는데 물리적인 저장장치 flash memory에 Flash device driver를 이용하여 데이터를
// 읽고 쓰거나 블록을 소거하는 일을 한다 (동영상 강의를 참조).
// flash memory에 데이터를 읽고 쓰거나 소거하기 위해서 fdevicedriver.c에서 제공하는 인터페이스를
// 호출하면 된다. 이때 해당되는 인터페이스를 호출할 때 연산의 단위를 정확히 사용해야 한다.
// 읽기와 쓰기는 페이지 단위이며 소거는 블록 단위이다.

int main(int argc, char *argv[])
{
	char sectorbuf[SECTOR_SIZE];
	char pagebuf[PAGE_SIZE];
	char sparebuf[SPARE_SIZE];
 	char *blockbuf;

	char op=*argv[1];
	char *fname=argv[2];
	int ppn;
	int blocknum;


	// flash memory 파일 생성: 위에서 선언한 flashfp를 사용하여 flash 파일을 생성한다. 그 이유는 fdevicedriver.c에서
	//                 flashfp 파일포인터를 extern으로 선언하여 사용하기 때문이다.
	if(op=='c'){
		flashfp=fopen(fname,"w");
		blocknum=atoi(argv[3]);

		blockbuf=(char *)malloc(blocknum*BLOCK_SIZE);

		memset((void *)blockbuf,(char)0xFF,blocknum*BLOCK_SIZE);

		fwrite(blockbuf,blocknum*BLOCK_SIZE,1,flashfp);

		fclose(flashfp);
		exit(0);
	}
	// 페이지 쓰기: pagebuf의 섹터와 스페어에 각각 입력된 데이터를 정확히 저장하고 난 후 해당 인터페이스를 호출한다
	if(op=='w'){
			char *sectordata=argv[4];
			char *sparedata=argv[5];
			ppn=atoi(argv[3]);
			flashfp=fopen(fname,"r+");
			int copyblock=0;
			int copypage;
			int a;
			int count=0;
			char *tempage;
			int w;

			fseek(flashfp,ppn*PAGE_SIZE,SEEK_SET);
			fread((void *)pagebuf,PAGE_SIZE,1,flashfp);
			for(int i=0;i<PAGE_SIZE;i++){
					if(pagebuf[i]==(char)0xFF) count++;

				}
			if(count==PAGE_SIZE) w=1;
			else w=0;

			//fill the pagebuf
			memset((void *)sectorbuf,(char)0xFF,SECTOR_SIZE);

			memset((void *)sparebuf,(char)0xFF,SPARE_SIZE);

			memcpy(sparebuf,sparedata,strlen(sparedata));

			memcpy(sectorbuf, sectordata,strlen(sectordata));

			memset((void *)pagebuf, (char)0xFF, PAGE_SIZE);

			memcpy(pagebuf,sectorbuf,SECTOR_SIZE);

			memcpy(pagebuf+SECTOR_SIZE,sparebuf,SPARE_SIZE);

			char *page=pagebuf;

			//find the block of ppn
			blocknum=ppn/4;
			copypage=ppn%4;

			blockbuf=(char *)malloc(BLOCK_SIZE);
			memset((void *)blockbuf,(char)0xFF,BLOCK_SIZE);


		/*	fseek(flashfp,blocknum*BLOCK_SIZE,SEEK_SET);
			fread((void *)blockbuf,BLOCK_SIZE,1,flashfp);

			for(int i=0;i<BLOCK_SIZE;i++){
					if(blockbuf[i]==(char)0xFF) count++;

				}
			if(count==BLOCK_SIZE) {

					a=dd_write(ppn,page);

				}*/

			if(w==1)	a=dd_write(ppn,page);
			else{
			//find empty block

			int b=0;
			int c;
			fseek(flashfp,0,SEEK_SET);
			while((c=fread((void *)blockbuf, BLOCK_SIZE, 1, flashfp))!=0){

				count=0;
				for(int i=0;i<BLOCK_SIZE;i++){
					if(blockbuf[i]==(char)0xFF) count++;

				}
				if(count==BLOCK_SIZE){
					copyblock=b;
					break;
				}
				b++;
			}

			//move block to empty block-//copy &pasre the block(blocknum)
				tempage=(char *)malloc(PAGE_SIZE);
				memset((void *)tempage,(char)0xFF,PAGE_SIZE);

				for(int i=0;i<PAGE_NUM;i++){
							if(i==copypage) continue;
							fseek(flashfp,i*PAGE_SIZE+blocknum*BLOCK_SIZE,SEEK_SET);
							fread((void *)tempage, PAGE_SIZE, 1, flashfp);
							fseek(flashfp,i*PAGE_SIZE+copyblock*BLOCK_SIZE,SEEK_SET);
							fwrite((void *)tempage, PAGE_SIZE, 1, flashfp);

					}

					fseek(flashfp,copyblock*BLOCK_SIZE,SEEK_SET);
					fread((void *)blockbuf, BLOCK_SIZE, 1, flashfp);




			//delete the block(blocknum)
			char *emptybuf;
			emptybuf=(char *)malloc(BLOCK_SIZE);
			memset((void *)emptybuf,(char)0xFF,BLOCK_SIZE);

			fseek(flashfp, blocknum*BLOCK_SIZE, SEEK_SET);
			fwrite((void *)emptybuf, BLOCK_SIZE, 1, flashfp);

			//Input pagedata
			a=dd_write(ppn,page);



			//move copy block to the block(blokc num)

			for(int i=0; i<PAGE_NUM;i++){
				if(i==copypage) continue;

				fseek(flashfp,copyblock*BLOCK_SIZE+i*PAGE_SIZE,SEEK_SET);
				fread((void *)pagebuf, PAGE_SIZE, 1, flashfp);
				fseek(flashfp,blocknum*BLOCK_SIZE+i*PAGE_SIZE,SEEK_SET);
				fwrite((void *)pagebuf, PAGE_SIZE, 1, flashfp);
			}

			//delete the copyblock
			fseek(flashfp,copyblock*BLOCK_SIZE,SEEK_SET);
			fwrite((void *)emptybuf, BLOCK_SIZE, 1, flashfp);
			fseek(flashfp,copyblock*BLOCK_SIZE,SEEK_SET);
			fread((void *)blockbuf, BLOCK_SIZE, 1, flashfp);

		}



			fclose(flashfp);
			exit(0);




	}
	// 페이지 읽기: pagebuf를 인자로 사용하여 해당 인터페이스를 호출하여 페이지를 읽어 온 후 여기서 섹터 데이터와
	//                  스페어 데이터를 분리해 낸다
	if(op=='r'){
		ppn=atoi(argv[3]);
		flashfp=fopen(fname,"r+");

		int a=dd_read( ppn, pagebuf);
		int cnt=0;
		memcpy(sectorbuf,pagebuf,SECTOR_SIZE);
		memcpy(sparebuf,pagebuf+SECTOR_SIZE,SPARE_SIZE);
		for(int i=0;i<PAGE_SIZE;i++){
			if(pagebuf[i]!=(char)0xFF) cnt++;
		}
		if(cnt!=0){
			for(int i=0;i<SECTOR_SIZE;i++){
				if(sectorbuf[i]!=(char)0xFF) printf("%c",sectorbuf[i]);
			}
			printf(" ");
			for(int i=0;i<SPARE_SIZE;i++){
				if(sparebuf[i]!=(char)0xFF) printf("%c",sparebuf[i]);
			}
			printf("\n");
			}
			fclose(flashfp);
			exit(0);
		}
	// memset(), memcpy() 등의 함수를 이용하면 편리하다. 물론, 다른 방법으로 해결해도 무방하다.
	if(op=='e'){
		ppn=atoi(argv[3]);
		flashfp=fopen(fname,"r+");
		int a= dd_erase(ppn);
		fclose(flashfp);
		exit(0);

	}
	return 0;
}

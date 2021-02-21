#include "ssufs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	/* 1 */
	int num=0;
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

	num=open_namei(filename);
	if(num!=-1) return -1;

	num=ssufs_allocInode();

	if(num==-1) return -1;
	memcpy(inode->name, filename, sizeof(filename));
	inode->status = INODE_IN_USE;
	inode->file_size = 0;
	ssufs_writeInode(num, inode);
	//ssufs_readInode(num, inode);
	//printf("%d:%c,%d,%s\n",num,inode->status,inode->file_size,filename);
	free(inode);

	return num;
}

void ssufs_delete(char *filename){
	/* 2 */
	int num;

	num=open_namei(filename);
	if(num!=-1)
		ssufs_freeInode(num);

}

int ssufs_open(char *filename){
	/* 3 */
	int num=-1;
	int cnt=0;
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	num=open_namei(filename);
	if(num==-1) return -1;

	for(int i=0;i<MAX_OPEN_FILES;i++){
		if(file_handle_array[i].inode_number==-1){
			file_handle_array[i].inode_number=num;
			file_handle_array[i].offset=0;
			break;
		}
		cnt++;
	}
	if(cnt==MAX_OPEN_FILES) num=-1;
	//ssufs_dump();
	free(inode);
	return num;
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	/* 4 */
	int off;
	int bcnt;
	int idx;
	char *newbuf=malloc(4*BLOCKSIZE);
	char *buf2=malloc(BLOCKSIZE);
	char *ans=malloc(4*BLOCKSIZE);
	char *buf3=malloc(4*BLOCKSIZE);
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

	ssufs_readInode(file_handle_array[file_handle].inode_number,inode);
	//check error 1. size
	if(file_handle_array[file_handle].offset+nbytes>4*BLOCKSIZE) return -1;
	if(file_handle_array[file_handle].offset+nbytes>inode->file_size) return -1;

	//find start point
	bcnt=file_handle_array[file_handle].offset/BLOCKSIZE;

	for(int i=bcnt;i<MAX_FILE_SIZE;i++){
		idx=inode->direct_blocks[i];
		ssufs_readDataBlock(idx,buf2);
		strcat(buf3,buf2);
	}

	off=file_handle_array[file_handle].offset%BLOCKSIZE;

	strncpy(ans,buf3+off,nbytes);

	//printf("ans:%s\n",ans);
	strncpy(buf3,ans,nbytes);
	//printf("buf3:%s\n",ans);
	strncpy(buf,buf3,nbytes);
	//printf("buf:%s\n",buf);
	file_handle_array[file_handle].offset+=nbytes;

	free(newbuf);
	free(buf2);
	free(buf3);
	free(ans);
	free(inode);
	return 0;
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	/* 5*/
	int bcnt;//count of essential data block
	int num=0;//count of free data block
	int cnt=0;
	int off=0;
	int length=0;
	int datanum;

	int pos=0;
	int size=0;
	char newbuf[MAX_FILE_SIZE][BLOCKSIZE]={0};
	char *emt=malloc(BLOCKSIZE);
	char *test=malloc(BLOCKSIZE*4);
	char *buf2=malloc(BLOCKSIZE*4);
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));


	//error check 1. length of writing buf
	if(file_handle_array[file_handle].offset+nbytes>4*BLOCKSIZE) return -1;

	//error check 2. free data block
	cnt=file_handle_array[file_handle].offset%BLOCKSIZE;

	if((cnt+nbytes)%BLOCKSIZE==0)
		bcnt=(cnt+nbytes)/BLOCKSIZE;
	else bcnt=(cnt+nbytes)/BLOCKSIZE+1;

	for(int i=0;i<NUM_INODES;i++){
		ssufs_readInode(i,inode);
		for(int j=0;j<4;j++){
			if(inode->direct_blocks[j]==-1) num++;
		}
	}
	ssufs_readInode(file_handle_array[file_handle].inode_number,inode);
	cnt=file_handle_array[file_handle].offset/BLOCKSIZE;
	if(inode->direct_blocks[cnt]!=-1) num++;
	if(num<bcnt) return -1;

	//error check 3. length of buf
	if(strlen(buf)<nbytes) return -1;

	//check offset
	if(file_handle_array[file_handle].offset%BLOCKSIZE==0){

		strncpy(test,buf,nbytes);

		strncpy(buf2,test,nbytes);

	}else {
		cnt=file_handle_array[file_handle].offset/BLOCKSIZE;
		off=file_handle_array[file_handle].offset%BLOCKSIZE;
		ssufs_readDataBlock(inode->direct_blocks[cnt],emt);
		strncpy(buf2,emt,off);
		strncat(buf2,buf,nbytes);
		if(file_handle_array[file_handle].offset+nbytes<inode->file_size)
			strncat(buf2,emt+off+nbytes,BLOCKSIZE-off-nbytes);
	}


	//split buf2 for BLOCKSIZE
	for(int i=0;i<MAX_FILE_SIZE;i++){
		strncpy(newbuf[i],buf2+i*BLOCKSIZE,BLOCKSIZE);
		if(i*BLOCKSIZE>=strlen(buf2)) {
			length=i;
			break;
		}
	}

	//write newbuf to datablock
	for(int i=0;i<length;i++){
		if(inode->direct_blocks[cnt]==-1){
			datanum=ssufs_allocDataBlock();
			if(datanum==-1) return -1;
		}else datanum=inode->direct_blocks[cnt];

		ssufs_writeDataBlock(datanum, newbuf[i]);
		inode->direct_blocks[cnt]=datanum;
		cnt++;
		if(cnt>MAX_FILE_SIZE) break;
	}
	cnt=0;

	for(int i=0;i<MAX_FILE_SIZE;i++){
		if(inode->direct_blocks[i]!=-1) cnt++;
	}

	inode->file_size=BLOCKSIZE*(cnt-1)+strlen(buf2);

	file_handle_array[file_handle].offset+=nbytes;


	ssufs_writeInode(file_handle_array[file_handle].inode_number,inode);
	free(emt);
	free(buf2);
	free(inode);
	return 0;
}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);

	int fsize = tmp->file_size;

	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {
		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}

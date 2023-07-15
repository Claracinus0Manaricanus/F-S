#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

char* getFile_FD(char*, int*);
int writeFile_FD(char*, char*);

int main(){
	//socket creation
	int sfd=socket(AF_INET,SOCK_STREAM,6);
	if(sfd==-1){
		printf("Socket creation error, exiting. ERRNO=%d\n",errno);
		return 1;
	}
	int yes=1;
	if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1){
		printf("setsockopt failed");
	}


	//binding
	uint16_t port=34576;

	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=(port<<8)+(port>>8);//port
	addr.sin_addr.s_addr=0;//127.0.0.1

	uint32_t sizeOA=sizeof(addr);
	if(bind(sfd,(struct sockaddr*)&addr,sizeOA)==-1){
		printf("binding failed, exiting. ERRNO=%d\n",errno);
		close(sfd);
		return 1;
	}


	//listen
	if(listen(sfd,SOMAXCONN)==-1){
		printf("listen failed, exiting. ERRNO=%d\n",errno);
		close(sfd);
		return 1;
	}


	//accept
	int cfd=accept(sfd,NULL,NULL);//get address info (todo)
	printf("client connected\n");


	//getting clients request
	char* cptr=malloc(1);
	recv(cfd,cptr,1,0);


	//variable
	char* filename=malloc(20);
	char* file=NULL;
	uint32_t fileSize=0;


	//deciding upon request
	if((*cptr)==0){//receive a file
		printf("receiving file.\n");
		//from client
		recv(cfd,filename,20,0);//filename
		printf("filename: %s\n",filename);
		recv(cfd,&fileSize,4,0);//filesize
		printf("filesize: %ubytes\n",fileSize);
		file=malloc(fileSize);
		recv(cfd,file,fileSize,0);//file
		//disk output
		writeFile_FD(filename,file);
	}else if((*cptr)==1){//send a file
		printf("file requested.\n");
		recv(cfd,filename,20,0);
		printf("sending %s\n",filename);
		file=getFile_FD(filename,&fileSize);
		send(cfd,&fileSize,4,0);
		send(cfd,file,fileSize,0);
	}


	//termination
	free(cptr);
	printf("\n");
	close(cfd);
	close(sfd);
	return 0;
}


char* getFile_FD(char* filename, int* size){//get a file from disk
	FILE* reader=fopen(filename,"r");
	if(reader==NULL)return NULL;

	fseek(reader,0,SEEK_END);
	(*size)=ftell(reader);
	rewind(reader);

	char* file=malloc((*size)+1);
	for(int i=0;i<(*size);i++){
		file[i]=fgetc(reader);
	}file[(*size)]=0;
	(*size)++;

	fclose(reader);
	return file;
}

int writeFile_FD(char* filename, char* fileData){
	FILE* writer=fopen(filename,"w");
	if(writer==NULL)return 1;
	fputs(fileData,writer);

	fclose(writer);
	return 0;
}
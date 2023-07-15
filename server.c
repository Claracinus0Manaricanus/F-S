#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

char* getFile_FD(char*, int*);
int writeFile_FD(char*, uint32_t, char*);

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
	int packets=0;


	//deciding upon request
	if((*cptr)==0){//receive a file
		printf("receiving file.\n");
		//from server
		printf("filename: ");scanf("%s",filename);
		send(cfd,filename,20,0);
		recv(cfd,&fileSize,4,0);//filesize
		printf("filesize: %ubytes\n",fileSize);
		file=malloc(fileSize);
		packets=fileSize/1024;packets++;
		printf("packets: %i",packets);
		for(int i=0;i<packets;i++){
			recv(cfd,&file[i*1024],1024,0);//file
		}
		for(int i=0;i<fileSize;i++){
			printf("%u,",file[i]);
		}
		writeFile_FD(filename,fileSize,file);
	}else if((*cptr)==1){//send a file
		printf("file requested.\n");
		recv(cfd,filename,20,0);
		printf("sending %s\n",filename);
		file=getFile_FD(filename,&fileSize);
		send(cfd,&fileSize,4,0);
		packets=fileSize/1024;packets++;
		printf("packets: %i",packets);
		for(int i=0;i<packets;i++){
			send(cfd,&file[i*1024],1024,0);
		}
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

	char* file=malloc((*size));
	for(int i=0;i<(*size);i++){
		file[i]=fgetc(reader);
	}

	fclose(reader);
	return file;
}

int writeFile_FD(char* filename, uint32_t size, char* fileData){
	FILE* writer=fopen(filename,"w");
	if(writer==NULL)return 1;
	for(int i=0;i<size;i++){
		fputc(fileData[i],writer);
	}

	fclose(writer);
	return 0;
}
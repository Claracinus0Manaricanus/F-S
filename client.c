#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

//optimize
int ipParser(char* input){//in put is expected to be 15 bytes
	char ip0[4],ip1[4],ip2[4],ip3[4];//ip0 is MSB(most significant byte)
	for(int ind=0,pos=0,i=0;i<15;i++){
		if(input[i]==0)break;
		if(input[i]=='.'){ind++;pos=0;continue;}
		switch(ind){
			case 0:ip0[pos]=input[i];break;
			case 1:ip1[pos]=input[i];break;
			case 2:ip2[pos]=input[i];break;
			case 3:ip3[pos]=input[i];break;
		}
		pos++;
	}
	ip0[3]=0;
	ip1[3]=0;
	ip2[3]=0;
	ip3[3]=0;

	return (atoi(ip0))+(atoi(ip1)<<8)+(atoi(ip2)<<16)+(atoi(ip3)<<24);
}

char* getFile_FD(char*,int*);
int writeFile_FD(char*, char*);

int main(){
	//variables to use
	uint16_t inport=0;
	uint16_t port=4231;
	uint32_t ip=0;
	char* ipToUse=malloc(15);


	//getting input
	printf("Please enter port:");scanf("%u",&inport);
	port=(inport<<8)+(inport>>8);
	printf("network byte order:%u\n",port);

	printf("Please enter ipv4 address:");
	scanf("%s",ipToUse);
	//converting ip input
	ip=ipParser(ipToUse);
	printf("network byte order:%u\n",ip);
	fgets(ipToUse,15,stdin);


	//socket creation
	int sfd=socket(AF_INET,SOCK_STREAM,6);
	if(sfd==-1){
		printf("Socket creation error, exiting. ERRNO=%d\n",errno);
		return 1;
	}


	//connecting
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=port;
	addr.sin_addr.s_addr=ip;//16777343

	uint32_t sizeOA=sizeof(addr);

	if(connect(sfd,(struct sockaddr*)&addr,sizeOA)==-1){
		printf("connection failed exiting. ERRNO=%d\n",errno);
		close(sfd);
		return 1;
	}


	//aquiring request (send or receive a file)
	char choice=0;
	printf("send:0 receive:1\n> ");
	scanf("%c",&choice);

	if((choice-48)>0)
		choice=1;
	else
		choice=0;

	
	//sending request
	send(sfd,&choice,1,0);


	//variable
	char* cptr=malloc(1);
	char* filename=malloc(20);
	char* file=NULL;
	uint32_t fileSize=0;


	//acting upon request
	if(choice==0){//send a file
		printf("sending file.\n");
		printf("filename: ");scanf("%s",filename);
		file=getFile_FD(filename,&fileSize);
		send(sfd,filename,20,0);
		send(sfd,&fileSize,4,0);
		send(sfd,file,fileSize,0);
	}else if(choice==1){//receive a file
		printf("receiving file.\n");
		//from server
		printf("filename: ");scanf("%s",filename);
		send(sfd,filename,20,0);
		recv(sfd,&fileSize,4,0);//filesize
		printf("filesize: %ubytes\n",fileSize);
		file=malloc(fileSize);
		recv(sfd,file,fileSize,0);//file
		//disk output
		writeFile_FD(filename,file);
	}


	//termination
	free(ipToUse);
	free(cptr);
	free(filename);
	free(file);
	printf("\n");
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
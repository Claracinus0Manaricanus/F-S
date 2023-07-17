#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#define PACKET_SIZE 8

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

	return (atoi(ip0))|(atoi(ip1)<<8)|(atoi(ip2)<<16)|(atoi(ip3)<<24);
}

char* getFile_FD(char*,int*);
int writeFile_FD(char*, uint32_t, char*);
int receiveFile(int,char*,uint32_t*,char**);
int sendFile(int,char*,uint32_t*,char**);

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
	char* filename=malloc(20);
	char* file=NULL;
	uint32_t fileSize=0;


	//acting upon request
	if(choice==0){//send a file
		sendFile(sfd,filename,&fileSize,&file);
	}else if(choice==1){//receive a file
		receiveFile(sfd,filename,&fileSize,&file);
		writeFile_FD(filename,fileSize,file);
	}


	//termination
	free(ipToUse);
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


int receiveFile(int sfd,char* filename,uint32_t* size,char** file){
	//send server the requested files name
	printf("filename: ");scanf("%s",filename);
	send(sfd,filename,20,0);
	//receive file size in bytes
	printf("receiving file.\n");
	recv(sfd,size,4,0);//filesize
	printf("filesize: %ubytes\n",(*size));
	(*file)=malloc((*size));//allocate memory for the file
	//receive in 1024 byte packets
	uint32_t packets=(*size)/PACKET_SIZE;packets++;
	printf("packets: %u",packets);
	for(int i=0;i<packets;i++){
		recv(sfd,&(*file)[i*PACKET_SIZE],PACKET_SIZE,0);//file
	}

	return 0;
}


int sendFile(int sfd,char* filename,uint32_t* size,char** fileData){
	printf("file requested.\n");
	printf("filename: ");scanf("%s",filename);
	send(sfd,filename,20,0);
	printf("sending %s\n",filename);
	(*fileData)=getFile_FD(filename,size);
	send(sfd,size,4,0);
	uint32_t packets=(*size)/PACKET_SIZE;packets++;
	printf("packets: %u",packets);
	for(int i=0;i<packets;i++){
		send(sfd,&(*fileData)[i*PACKET_SIZE],PACKET_SIZE,0);
	}

	return 0;
}
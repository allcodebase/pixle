// automate the client ID number for printing purpose
// declare or define anything ONLY when needed 

#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define N 2
#define PORT 8787
#define BUF_SIZE 1024

typedef struct data {
  int socket_fd;
}p_address;

void *rcvmsg(void *ptr) {
	int sockfd = ((p_address *)ptr)->socket_fd;
	char buffer[BUF_SIZE];
	bool flag = true;
	while(flag) {
		if(recv(sockfd,buffer,BUF_SIZE,0) < 0) {
			perror("Error : recv failed inside rcvmsg function\n");
			flag = false;
		}else {
			printf("%s\n",buffer);
		}
	}
	free(ptr);
	exit(1);
}
int main(int argc,char **argv) {
	/*

		struct sockaddr_in {
		    short int          sin_family;  // Address family, AF_INET
		    unsigned short int sin_port;    // Port number
			struct in_addr     sin_addr;    // Internet address ***
		    unsigned char      sin_zero[8]; // Same size as struct sockaddr
		};


		struct sockaddr {
    		unsigned short    sa_family;    // address family, AF_xxx
    		char              sa_data[14];  // 14 bytes of protocol address
		};

	*/
	
	if(argc < 2) {
		perror("usage : client < server_ip_address > : ");
		exit(1);
	}
	
	// create socket
	
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0) {
		perror("socket creation failed in client side : ");
		exit(1);
	}else printf("socket created in the client_1 side. \n");
	
	// fill socket data structure
	struct sockaddr_in addr;
	char *server_address = argv[1];
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family = AF_INET;  
	addr.sin_addr.s_addr = inet_addr(server_address);
	addr.sin_port = PORT;     
	if(connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {  
		perror("Error connecting to the server :\n");  
		exit(1);  
	}else printf("Connected to the server...\n");
 
	// create thread for read data
	pthread_t client_1;

	
 	data * p = (data *)malloc(sizeof(data));
 	p->socket_fd = sockfd;
	pthread_create(&client_1,NULL,rcvmsg,(void*)p);
	if(pthread_detach(client_1)) {
		perror("Error : in pthread_detach \n");
	}
	// write data send logic
	char *buffer = (char*)malloc(BUF_SIZE*sizeof(char));
	while(true) {
		fgets(buffer,BUF_SIZE,stdin);
		if('\n' == buffer[strlen(buffer) - 1]) buffer[strlen(buffer) - 1] = '\0';
		if(send(sockfd,buffer,strlen(buffer),0) < 0) {
			perror("Error ending data.\n");
		}
	}

	exit(1);
}



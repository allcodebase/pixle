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
#define W_Queue 5

#define MAX_CLIENT_COUNT 5

typedef struct data {
	int socket_fd;
}p_address;

int new_sockfd[MAX_CLIENT_COUNT];

void * handle_client(void *ptr) {
	int *p = (int *)(ptr);
	int sockfd = *p;
	char buffer[BUF_SIZE];
	memset(buffer,0,BUF_SIZE);
	bool FLAG = false;
	while(!FLAG) {
		if(recv(sockfd,buffer,BUF_SIZE,0) < 0) {
			perror("Error : recv failed inside rcvmsg function\n");
			FLAG = true; //  ????
		}else {
			if(send(sockfd,buffer,strlen(buffer),0) < 0) {
				perror("Error ending data.\n");
			}else {
				// printf("%s\n", );
			}
		}
	}
	close(sockfd);
	exit(1);
}

int main() {
	
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

	int sockfd_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd_listen < 0) {
		perror("Error creating listening socket \n");
		exit(1);
	}else printf("Listening socket created in the server side\n");
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = PORT;
	if (bind(sockfd_listen, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("Error binding!\n");
		exit(1);
	}else printf("Binding done with PORT : %d\n",PORT);
	
	printf("waiting for a connection from client side\n");
	listen(sockfd_listen, W_Queue);

	struct sockaddr_in cl_info_addr;
	int len = sizeof(cl_info_addr);
	int client_count = 0;
	bool LISTEN_FLAG_SERVER = true;
	
								// main thread is polling connections using the listen_socket
	while(LISTEN_FLAG_SERVER) { // check new connections ....
		
		new_sockfd[client_count] = accept(sockfd_listen, (struct sockaddr *) &cl_info_addr, (socklen_t *)&len);
		
		if (new_sockfd[client_count] < 0) {
			printf("Error accepting connection!\n");
			exit(1);
		}else {
			char clientAddr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,&(cl_info_addr.sin_addr),clientAddr,INET_ADDRSTRLEN);
			printf("Connection accepted from %s\n", clientAddr); 			
			pthread_t rThread;
			if(pthread_create(&rThread, NULL,handle_client, (void *)(new_sockfd+client_count)) ) {
				perror("ERROR: pthread failed inside server_1\n");
				exit(1);
			}
			if(pthread_detach(rThread)){
				perror("ERROR: pthread_detach failed inside server_1\n");
				exit(1); // try something else...not to completely end this server process
			}
			
			client_count++;
		}

		if(client_count == MAX_CLIENT_COUNT) LISTEN_FLAG_SERVER = false;
	}

	close(sockfd_listen);
	exit(1);
}

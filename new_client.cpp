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
#define N 			2
#define PORT 		8787
#define BUF_SIZE	1024
#define ID_SIZE		10

#define ID_SENT		0 

#define MT_LOGIN	0
#define MT_CONV 	1


typedef struct data {
  int socket_fd;
}p_address;
char DELIM = ':';
unsigned int client_status = 0; // this will act as flag

int main(int argc,char **argv) {
	/*======================================S0==============================================*/
	if(argc < 2) {
		perror("usage : client < server_ip_address > : ");
		exit(1);
	}
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0) {
		perror("socket creation failed in client side : ");
		exit(1);
	}else printf("socket created in the client_1 side. \n");
	/*======================================S1==============================================*/
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

										/*<><><><><><><>*/
	/*=======================================S3==============================================*/
	// send login information
	//
	char *buffer = (char *)malloc(BUF_SIZE*sizeof(char));
	char my_id[ID_SIZE],op_id[ID_SIZE];
	printf("your ID \n"); scanf("%s",my_id);
	printf("opponent ID ? \n"); scanf("%s",op_id); // asking for the opponent
	buffer[0] = buffer[0] | (1<< MT_LOGIN);
	int idx = 1;
	for(int i=0;i<strlen(my_id);i++) buffer[idx++] = my_id[i];
	buffer[idx++] = DELIM;
	for(int i=0;i<strlen(op_id);i++) buffer[idx++] = op_id[i];
	buffer[idx] = DELIM; 

	if(send(sockfd,buffer,strlen(buffer),0) < 0) {
		perror("Error ending data.\n");
	}else {
		printf("data sent to server_1\n");
	}
	memset(buffer,0,BUF_SIZE);
	if(recv(sockfd,buffer,BUF_SIZE,0) < 0) { // waiting and recieve the ok code for the game to start
		perror("Error : recv failed inside rcv_data_from_server function\n");
	}else {
		printf("data received from server_1 = %d\n",buffer[0]);
	}

	/*=======================================S6==============================================*/

	bool turn = (buffer[0] == 1)?true:false;
	if(turn) printf("Its my turn\n");

	// talk
	// clear the stdin
	char c;
	while ((c = getchar()) != '\n' && c != EOF) { }
	

	while(true) {

		memset(buffer,0,BUF_SIZE);
		
		if(turn) {
			printf("you ...  ");
			fgets(buffer,BUF_SIZE,stdin);
			if('\n' == buffer[strlen(buffer) - 1]) buffer[strlen(buffer) - 1] = '\0';
			if(send(sockfd,buffer,strlen(buffer),0) < 0) {
				perror("Error ending data.\n");
			}
			turn ^= 1;
		}else {
			if(recv(sockfd,buffer,BUF_SIZE,0) < 0) {
				perror("Error : recv failed inside rcv_data_from_server function\n");
			}else {
				printf("opponent : %s\n",buffer);
			}
			turn ^= 1;
		}
	}	
	exit(1);
}



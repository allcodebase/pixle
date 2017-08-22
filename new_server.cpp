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
using namespace std;
#define N 2
#define PORT 8787
#define BUF_SIZE 1024
#define W_Queue 5

#define MAX_CLIENT_COUNT 20
#define ID_SENT		0

#define MT_LOGIN	0
#define MT_CONV 	1

typedef struct data {
	int socket_fd;
}p_address;

struct client_infos{
	int ID_1;
	int ID_2;
	int sockfd_1;
	int sockfd_2;
	bool turn;
};

int new_sockfd[MAX_CLIENT_COUNT];

map<int, int > ID_SOCK_PAIR;
set<int> waiting_for;
map<int, int> match;
char DELIM = ':';


void * handle_game(void *ptr) {
	client_infos *p = (client_infos *)(ptr);
	char buffer[BUF_SIZE];
	memset(buffer,0,BUF_SIZE);
	bool _turn = p->turn;
	printf("Game created\n");
	while(true) {
		memset(buffer,0,BUF_SIZE);
		if(!_turn) {
			if(recv(p->sockfd_1,buffer,BUF_SIZE,0) < 0) {
				perror("Error :\n");
			}else {
				printf("data received from %d : %s\n",p->ID_1,buffer);
			}
			if(send(p->sockfd_2,buffer,strlen(buffer),0) < 0) {
				perror("Error ending data.\n");
			}else {
				printf("data sent to %d : %s\n",p->ID_2,buffer);
			}
			_turn ^= 1;
		}else {
			if(recv(p->sockfd_2,buffer,BUF_SIZE,0) < 0) {
				perror("Error :\n");
			}else {
				printf("data received from %d : %s\n",p->ID_2,buffer);
			}
			if(send(p->sockfd_1,buffer,strlen(buffer),0) < 0) {
				perror("Error ending data.\n");
			}else {
				printf("data sent to %d : %s\n",p->ID_1,buffer);
			}
			_turn ^= 1;
		}
	}
	close(p->sockfd_1);
	close(p->sockfd_2);
	exit(1);
}

void process_buffer(char *,int *,int *);
int main() {	
	srand(time(NULL));
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
	
	pthread_t thread_pool[MAX_CLIENT_COUNT/2];
	int thread_count = 0;
	char buffer[BUF_SIZE];
	memset(buffer,0,BUF_SIZE);

								// main thread is polling connections using the listen_socket
	while(LISTEN_FLAG_SERVER) { // check new connections ....
		new_sockfd[client_count] = accept(sockfd_listen, (struct sockaddr *) &cl_info_addr, (socklen_t *)&len);
		if(new_sockfd[client_count] < 0) {
			printf("Error accepting connection!\n");
			exit(1);
		}else {
			char clientAddr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,&(cl_info_addr.sin_addr),clientAddr,INET_ADDRSTRLEN);
			printf("Connection accepted from %s\n", clientAddr); 	
		}
		memset(buffer,0,sizeof(buffer));
		if(recv(new_sockfd[client_count],buffer,BUF_SIZE,0) < 0) {
			perror("Error : recv failed inside rcvmsg function\n");
		}else {
			int p1,p2;
			process_buffer(buffer,&p1,&p2);
			ID_SOCK_PAIR[p1] = new_sockfd[client_count];
			set<int>::iterator ptr;
			if(( ptr = waiting_for.find(p2) ) != waiting_for.end()) {
				printf("match found\n");
				memset(buffer,0,sizeof(BUF_SIZE));
				buffer[0] = 1;
				if(send(ID_SOCK_PAIR[p2],buffer,strlen(buffer),0) < 0) { // sending the data
					perror("Error ending data.\n");
				}
				memset(buffer,0,sizeof(BUF_SIZE));
				buffer[0] = 2;
				if(send(ID_SOCK_PAIR[p1],buffer,strlen(buffer),0) < 0) { // sending the data
					perror("Error ending data.\n");
				}
				waiting_for.erase(ptr);

				client_infos *ptr = (client_infos *)malloc(sizeof(client_infos));
				ptr->ID_1 = p2;
				ptr->ID_2 = p1;
				ptr->sockfd_1 = ID_SOCK_PAIR[p2];
				ptr->sockfd_2 = ID_SOCK_PAIR[p1];
				ptr->turn = 0;
				printf("turn = %d\n",ptr->turn);
				if(pthread_create(&thread_pool[thread_count], NULL,handle_game, (void *)(ptr)) ) {
					perror("ERROR: pthread failed inside server_1 exiting now !!\n");
					exit(1);
				}else {
					thread_count++;
				}
				if(pthread_detach(thread_pool[thread_count-1])){
					perror("ERROR: pthread_detach failed inside server_1\n");
					exit(1); // try something else...not to completely end this server process
				}
			}else {

				waiting_for.insert(p1);	
			}
				
		}

		client_count++;

		if(client_count == MAX_CLIENT_COUNT) LISTEN_FLAG_SERVER = false;
	}

	close(sockfd_listen);
	exit(1);
}

void process_buffer(char * buffer,int *p1,int *p2) {
	char c; int idx = 1,num = 0;
	switch(buffer[0]) {
		case (1<<MT_LOGIN):
			while((c = buffer[idx++]) != DELIM) {
				num = num*10 + (c - '0');
			}
			*p1 = num;
			num = 0;
			while((c = buffer[idx++]) != DELIM) {
				num = num*10 + (c - '0');
			}
			*p2 = num;
			break;

		default:
			;

	}
}

#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#define PORT 8080

void *connectionHandler(int *, int *);

int main() {
	
	int socket_desc;
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	
	if (socket_desc == -1) {
		printf("Could not create socket");
		return 1;
	}

	struct sockaddr_in server;

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

	//Bind and check for errors
	if ( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("Binding failed\n");
		return 1;
	} else {
		printf("Binded to port %d\n", PORT);
	}

	
	if (listen(socket_desc , 3) < 0) {
		printf("Failed starting to listen");
		return 1;
	} else {
		puts("Listening...");
	}

	size_t addr_len = sizeof(server);

	int activeConns = 0;
	char *welcomeMessage = "Connection established\n";

	int conn_socket;
	
	while (conn_socket = accept(socket_desc, (struct sockaddr *)&server, (socklen_t *)&addr_len)) {
		
		puts("Connection accepted");
		
		write(conn_socket , welcomeMessage , strlen(welcomeMessage));



		pthread_t thread_id; 

		struct arg_struct {
			int* arg1;
			int* arg2;
		};

		struct arg_struct args;

		args.arg1 = malloc(1);
		args.arg1 = conn_socket;

		args.arg2 = activeConns;

		if( pthread_create( &thread_id , NULL ,  connectionHandler , (void *)&args) < 0) {
			puts("Could not create thread");
			return 1;
		}

		pthread_join( thread_id , NULL);

	}

	return 0;

}

void *connectionHandler(int *conn_sock, int *activeConns) {
	int sock = *(int*)conn_sock;

	activeConns++;

	printf("there are %d connections\n", &activeConns);

	char *message;

	message = "There are x connections\n";
	write(sock, message, strlen(message));

	int read_size;
	char client_message[2000];

	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0)
	{

		//Send the message back to client
		write(sock , client_message , strlen(client_message));

	}

	//Disconnected
	if (read_size == 0) {
		puts("Client disconnected");
	} else {
		puts("recv failed");
	}

	//Free the pointer (caused segmentation fault all of a sudden?)
	//free(conn_sock);

	return 0;

}

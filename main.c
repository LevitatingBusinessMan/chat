#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#define PORT 8080

void *connectionHandler(void *);

struct arg_struct {
	int sock;
};

int activeConns;

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

		struct arg_struct args;

		args.sock = conn_socket;

		pthread_t thread_id;
		if( pthread_create( &thread_id , NULL ,  connectionHandler , (void *) &args) < 0) {
			puts("Could not create thread");
			return 1;
		}

		pthread_detach( thread_id );

	}

}

void *connectionHandler(void* arg_ptr) {
	struct arg_struct* args = arg_ptr;
	int sock = args->sock;

	activeConns++;

	char message[250];
	snprintf(message, 200, "There are %d people connected\nName: ", activeConns);
	write(sock, message, strlen(message));

 	int read_size;
	char client_message[200];

	char name[20];
	int nameSet = 0;

	while( (read_size = read(sock , client_message , 200)) > 0)
	{

		//Remove last character, which is a newline char.
		client_message[strlen(client_message)-1] = 0;

		if (nameSet == 0) {
			if (strlen(client_message) > 20) {
				strcpy(message, "Name is too long!\nName:");
				write(sock, message, strlen(message));
			} else {
				strcpy(name, client_message);
				snprintf(message, 200, "Identified as %s\n", name);
				write(sock, message, strlen(message));
				nameSet = 1;
			}
		} else {
			//Send the message back to client
			snprintf(message, 200, "[%s] %s\n", name, client_message);
			write(sock , message , strlen(message));
		}

	}

	//Disconnected
	if (read_size == 0) {
		snprintf(message, 200, "User %s disconnected", nameSet ? name : "Unknown" );
		puts(message);
	} else {
		puts("read failed");
	}

	activeConns--;
	return 0;

}

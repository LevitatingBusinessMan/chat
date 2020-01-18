#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
//#define PORT 8080
#define maxConnections 10

void *connectionHandler(void *);

struct arg_struct {
	int sock;
};

int activeConns;

int lsockets[maxConnections];

int* nextSocketSlot() {
	int i;
	for (i=0; i < 10; i++) {
    	if (lsockets[i] == 0)
			return &lsockets[i];
  	}
	perror("Somehow no space in lsockets array");
   	exit(EXIT_FAILURE);
}

void removeSocket(int* sock) {
	int i;
	for (i=0; i < 10; i++) {
    	if (lsockets[i] == *sock)
			lsockets[i] = 0;
  	}
}

int main(int argc, char *argv[]) {

	int port = 7966;
	int option;
	while ((option = getopt(argc, argv, "p:")) != -1 )  {  
        switch(option) {  
			case 'p':
				port = atoi(optarg);
				break;
        }
    } 

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
	server.sin_port = htons(port);

	//Bind and check for errors
	if ( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("Binding failed\n");
		return 1;
	} else {
		printf("Binded to port %d\n", port);
	}

	
	if (listen(socket_desc , 3) < 0) {
		printf("Failed starting to listen");
		return 1;
	} else {
		puts("Listening...");
	}

	size_t addr_len = sizeof(server);

	int conn_socket;
	
	while ((conn_socket = accept(socket_desc, (struct sockaddr *)&server, (socklen_t *)&addr_len))) {
		
		if (maxConnections < activeConns + 1) {
			puts("Connection denied (maxconns)");
			char *message = "Room is full!";
			write(conn_socket , message , strlen(message));
			return 0;
		}

		puts("Connection accepted");
		
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

//Send message to all (other) sockets in lsockets
void sendMessageToAll(int* originSock, char* message) {
	printf(message);
	int i;
	for (i=0; i < maxConnections; i++) {
    	if (lsockets[i] != 0 && lsockets[i] != *originSock) {
			write(lsockets[i], message, strlen(message));
		}
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

	while( (read_size = read(sock , client_message , sizeof(client_message))) > 0)
	{

		//Remove last character, which is a newline char.
		client_message[strlen(client_message)-1] = 0;

		if (nameSet == 0) {
			if (strlen(client_message) > 20) {
				strcpy(message, "Name is too long!\nName:");
				write(sock, message, strlen(message));
			} else {
				strcpy(name, client_message);
				snprintf(message, sizeof(message), "\033[32;1mIdentified as %s\033[0m\n", name);
				write(sock, message, strlen(message));
				snprintf(message, sizeof(message), "\033[32;1mUser connected as %s\033[0m\n", name);
				sendMessageToAll(&sock, message);
				nameSet = 1;
				
				//Register to socket array to receive messages
				int *slot = nextSocketSlot();
				*slot = sock;
			}
		} else {
			//Send the message to all other clients
			snprintf(message, sizeof(message), "\033[1m[%s]\033[0m %s\n", name, client_message);
			sendMessageToAll(&sock, message);
		}

		//Empty again
		memset(client_message, 0, sizeof(client_message));

	}

	//Disconnected
	if (read_size == 0) {
		snprintf(message, sizeof(message), "\033[91;1mUser %s disconnected\033[0m\n", nameSet ? name : "Unknown" );
		sendMessageToAll(&sock, message);
	} else {
		puts("Reading client message failed.");
	}

	activeConns--;

	removeSocket(&sock);

	return 0;

}

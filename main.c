#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define PORT 8080

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
	
	char socket_reply[2000];

	while (conn_socket = accept(socket_desc, (struct sockaddr *)&server, (socklen_t *)&addr_len)) {
		
		puts("Connection accepted");
		
		write(conn_socket , welcomeMessage , strlen(welcomeMessage));

	}

	return 0;

}

#include "k_srv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

struct K_server *create_server(
	int domain, int port, int service, int protocol, int backlog, u_long interface, void(*launch)(struct K_server *server)
){

	struct K_server *server = malloc(sizeof(struct K_server));
	if (!server) {
		perror("Failed to allocate memory for server... \n");
		exit(EXIT_FAILURE);
	}

	server->domain = domain;
	server->port = port;
	server->service = service;
	server->protocol = protocol;
	server->backlog = backlog;

	server->address.sin_family = domain;
	server->address.sin_port = htons(port);
	server->address.sin_addr.s_addr = htonl(interface);

	server->socket = socket(domain, service, protocol);
	if (server->socket < 0) {
		perror("Failed to create socket... \n");
		exit(EXIT_FAILURE);
	}

	if (bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address)) < 0) {
		perror("Failed to bind socket... \n");
		exit(EXIT_FAILURE);
	}

	if (listen(server->socket, backlog) < 0) {
		perror("Failed to listen on socket... \n");
		exit(EXIT_FAILURE);
	}

	server->launch = launch;

	return server;

};



void launch(struct K_server *server){
	char buffer[BUFFER_SIZE];

	while(1){
		printf("+===<- Waiting For Connection ->===+\n");
		socklen_t addrlen = sizeof(server->address);
		int new_socket = accept(server->socket, (struct sockaddr *)&server->address, &addrlen);
		ssize_t bytes = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
		if (bytes >= 0) {
			buffer[bytes] = '\0';  // Null-terminate the received data
			printf("Received request:\n%s\n", buffer);
			puts(buffer);
		} else {
			perror("+===<- Error reading buffer... ->===+");
		}
		char *response = "HTTP/1.1 200 OK\r\n"
						 "Content-Type: text/html; charset=UTF-8 \r\n\r\n"
						 "<!DOCTYPE html>\r\n"
						 "<html>\r\n"
                         "<head>\r\n"
                         "<title>Testing Basic HTTP-SERVER</title>\r\n"
                         "</head>\r\n"
                         "<body>\r\n"
                         "<h1>Hello kurai-webserver!</h1>\r\n"
                         "</body>\r\n"
                         "</html>\r\n";

		write(new_socket,response, strlen(response));
		close(new_socket);
	}
};
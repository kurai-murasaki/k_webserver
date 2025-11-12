/**
* @file k_srv.h
* @brief 
* @author Kurai
* @date 2025-11-06
*
* @copyright Copyright (c) 2025 Kurai Murasaki
* @license NotForSaleNotForYou - See LICENSE file for details
*
* @details 
*/

#ifndef K_SRV_H
#define K_SRV_H

/**
* Include files organization:
* 1. Standard C library headers
* 2. External third-party dependencies
* 3. Internal library headers
*/


// Standard library includes


// External dependencies
#include <netinet/in.h>

// Internal library includes


/**
* @brief Buffer size definition
*/
#define BUFFER_SIZE 4096


/**
* @struct K_server
* @brief Server structure
* @details This structure holds all the necessary information for the server.
*/
struct K_server{
	int domain;
	int port;
	int service;
	int protocol;
	int backlog;
	uint32_t interface;

	int socket;
	struct sockaddr_in address;

	void(*launch)(struct K_server *server);
};


/**
* @name create_server()
* @brief Create a new server instance
* @param domain The domain for the server (e.g., AF_INET)
* @param port The port number for the server
* @param service The service type (e.g., SOCK_STREAM)
* @param protocol The protocol type (e.g., IPPROTO_TCP)
* @param backlog The maximum length of the queue for pending connections
* @param interface The network interface to bind the server to
* @param launch The function to launch the server
* @return A pointer to the newly created K_server instance
*
* @details This function initializes a K_server instance with the provided parameters,
*          creates a socket, binds it to the specified address, and starts listening
*          for incoming connections.
*/
struct K_server *k_create_server(int domain, int port, int service, int protocol, int backlog, uint32_t interface, void(*launch)(struct K_server *server));


/**
* @name launch()
* @brief Launch the server
* @param server Pointer to the K_server instance
* @return void
*/
void k_launch(struct K_server *server);


#endif // K_SRV_H
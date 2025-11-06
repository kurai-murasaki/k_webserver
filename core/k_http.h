/**
* @file k_http.h
* @brief HTTP request and response handling
* @author Kurai
* @date 2025-11-06
*
* @copyright Copyright (c) 2025 Kurai Murasaki
* @license NotForSaleNotForYou - See LICENSE file for details
*
* @details This file contains the definitions and functions for handling HTTP requests and responses.
*/

#ifndef K_HTTP_H
#define K_HTTP_H

/**
* Include files organization:
* 1. Standard C library headers
* 2. External third-party dependencies
* 3. Internal library headers
*/


// Standard library includes
#include <stddef.h>

// External dependencies


// Internal library includes





/**
* @struct K_httpReq
* @brief HTTP request structure
* @details This structure holds the details of an HTTP request.
* @param method HTTP method (e.g., GET, POST)
* @param uri Request URI
* @param version HTTP version (e.g., HTTP/1.1)
* @param body Request body
* @param headers Array of HTTP headers
* @param header_count Number of headers
*/
typedef struct {
	char *method;
	char *uri;
	char *version;
	char *body;
	struct k_header {
		char *key;
		char *value;
	} *headers;
	size_t header_count;
} K_httpReq;




/**
* @struct K_httpRes
* @brief HTTP response structure
* @details This structure holds the details of an HTTP response.
* @param status_code HTTP status code (e.g., 200, 404)
* @param body Response body
* @param headers Array of HTTP headers
* @param header_count Number of headers
*/
typedef struct {
	int status_code;
	char *body;
	struct k_header *headers;
	size_t header_count;
} K_httpRes;





// === Helper functions ===


/**
* @name k_http_res_set_header()
* @brief Set a header in the HTTP response.
* @param res Pointer to the HTTP response structure.
* @param key Header field name.
* @param value Header field value.
* @return None
*/
void k_http_res_set_header(K_httpRes *res, const char *key, const char *value);


/**
* @name k_http_res_free()
* @brief Free the resources used by the HTTP response.
* @param res Pointer to the HTTP response structure.
* @return None
*/
void k_http_res_free(K_httpRes *res);


/**
* @name k_http_res_send()
* @brief Send the HTTP response to the client.
* @param client_fd File descriptor of the client socket.
* @param res Pointer to the HTTP response structure.
* @return 0 on success, -1 on failure.
*/
int k_http_res_send(int client_fd, K_httpRes *res);



#endif // K_HTTP_H
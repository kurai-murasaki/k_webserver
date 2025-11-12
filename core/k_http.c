#include "k_http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>


#define MAX_LINE_LENGTH 8192

//================================
//	 --- Request Parsing --- 
//================================

K_httpReq *k_http_req_parse(const char *raw_request, size_t raw_req_len){
	if(!raw_request || raw_req_len == 0){
		return NULL;
	}

	K_httpReq *req = calloc(1, sizeof(K_httpReq));
	if(!req){
		return NULL;
	}
	// Parse request line
	const char *line_end = memmem(raw_request,raw_req_len, "\r\n", 2);
	if(!line_end){ 
		return req;
	}
	size_t line_len = line_end - raw_request;
	char *request_line = strndup(raw_request, line_len);
	if(!request_line) {
		free(req);
		return NULL;
	}

	char *method = strtok(request_line, " ");
	char *uri = strtok(NULL, " ");
	char *version = strtok(NULL, " ");

	req->method = method ? strdup(method) : strdup("GET");
	req->uri = uri ? strdup(uri) : strdup("/");
	req->version = version ? strdup(version) : strdup("HTTP/1.1");

	free(request_line);

	// Parse headers
	const char *header_start = line_end + 2;
	const char *body_start = memmem(header_start, raw_req_len - (header_start - raw_request), "\r\n\r\n", 4);
	if (!body_start) {
		body_start = raw_request + raw_req_len;
	}

	// Count headers
	size_t header_count = 0;
	const char *curr = header_start;
	while(curr < body_start){
		const char *next = memmem(curr, body_start - curr, "\r\n", 2);
		if(!next || next == curr) break;
		if(memchr(curr, ':', next - curr)) header_count++;
		curr = next + 2;
	}

	req->header_count = header_count;
	if(header_count > 0) req->headers = calloc(header_count, sizeof(K_httpHeader));

	curr = header_start;
	size_t index = 0;
	while(curr < body_start && index < header_count){
		const char *next_line = memmem(curr, body_start - curr, "\r\n", 2);
		if(!next_line) break;

		size_t line_length = next_line - curr;
		char *header_line = strndup(curr, line_length);
		if(!header_line) { curr = next_line + 2; continue; }

		char *colon_pos = strchr(header_line, ':');
		if (colon_pos) {
			*colon_pos = '\0';
			req->headers[index].key = strdup(header_line);

			char *raw_value = colon_pos + 1;
			while (*raw_value == ' ') raw_value++;  // skip spaces but keep valid pointer
			req->headers[index].value = strdup(raw_value);
			index++;
		}



		free(header_line);
		curr = next_line + 2;
	}

	// Parse body
	body_start += 4; // Move past the "\r\n\r\n"
    if ((size_t)(raw_request + raw_req_len - body_start) > 0) {
        req->body_length = raw_request + raw_req_len - body_start;
        req->body = malloc(req->body_length + 1);
        if (req->body) {
            memcpy(req->body, body_start, req->body_length);
            req->body[req->body_length] = '\0';
        }
    }
	return req;
}

void k_http_req_free(K_httpReq *req){
	if(!req) return;

	free(req->method);
	free(req->uri);
	free(req->version);
	free(req->body);

	for(size_t i = 0; i < req->header_count; i++){
		free(req->headers[i].key);
		free(req->headers[i].value);
	}
	free(req->headers);
	free(req);
}

//================================================
//	 --- Response Handling --- 
//================================================


K_httpRes *k_http_res_create(void){
	K_httpRes *res = calloc(1, sizeof(K_httpRes));
	if(!res) return NULL;
	res->status_code = 200;
	return res;
}


void k_http_res_set_body(K_httpRes *res, const char *body, size_t len){
	if(!res) return;

	free(res->body);
	if(!body || len == 0){
		res->body = NULL;
		res->body_length = 0;
		return;
	}
	res->body = malloc(len + 1);
	if(res->body) {
		memcpy(res->body, body, len);
		res->body[len] = '\0';
		res->body_length = len;
	}
}


void k_http_res_set_header(K_httpRes *res, const char *key, const char *value){
	if(!res || !key || !value) return;
	K_httpHeader *tmp = realloc(res->headers, sizeof(*res->headers) * (res->header_count + 1));
	if(!tmp) return; //allocation failed
	res->headers = tmp;
	res->headers[res->header_count].key = strdup(key);
	res->headers[res->header_count].value = strdup(value);
	res->header_count++;
}

void k_http_res_free(K_httpRes *res){
	if(!res) return;
	free(res->body);

	for(size_t i = 0; i < res->header_count; i++){
		free(res->headers[i].key);
		free(res->headers[i].value);
	}
	free(res->headers);
}

// handle partial sends
int k_http_res_send(int client_socket, K_httpRes *res) {
	if (!res) return -1;

	// Calculate required buffer size
	size_t buffer_size = 128; // status line
	for (size_t i = 0; i < res->header_count; i++)
		buffer_size += strlen(res->headers[i].key) + strlen(res->headers[i].value) + 4;
	buffer_size += 4; // \r\n between headers and body
	buffer_size += res->body_length;

	char *buffer = malloc(buffer_size);
	if (!buffer) return -1;

	int len = snprintf(buffer, buffer_size, "HTTP/1.1 %d OK\r\n", res->status_code);
	for (size_t i = 0; i < res->header_count; i++)
		len += snprintf(buffer + len, buffer_size - len, "%s: %s\r\n",
						res->headers[i].key, res->headers[i].value);
	len += snprintf(buffer + len, buffer_size - len, "\r\n");

	if (res->body && res->body_length > 0) {
		memcpy(buffer + len, res->body, res->body_length);
		len += res->body_length;
	}

	size_t total_sent = 0;
	while (total_sent < (size_t)len) {
		ssize_t sent = send(client_socket, buffer + total_sent, len - total_sent, 0);
		if (sent <= 0) { free(buffer); return -1; }
		total_sent += sent;
	}

	free(buffer);
	return 0;
}
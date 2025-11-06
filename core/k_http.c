#include "k_http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>


//================================
//	 --- Request Parsing --- 
//================================

K_httpReq *k_http_req_parse(const char *raw_request){
	if(!raw_request){
		return NULL;
	}

	K_httpReq *req = calloc(1, sizeof(K_httpReq));
	if(!req){
		return NULL;
	}

	const char *line_end = strstr(raw_request, "\r\n");
	if(!line_end){ 
		return req;
	}

	char *request_line = strndup(raw_request, line_end - raw_request);
	char *method = strtok(request_line, " ");
	char *uri = strtok(NULL, " ");
	char *version = strtok(NULL, " ");

	req->method = method ? strdup(method) : strdup("GET");
	req->uri = uri ? strdup(uri) : strdup("/");
	req->version = version ? strdup(version) : strdup("HTTP/1.1");

	free(request_line);

	const char *header_start = line_end + 2;
	const char *body_start = strstr(header_start, "\r\n\r\n");
	if (!body_start) {
		body_start = header_start + strlen(header_start);
	}

	size_t header_count = 0;
	const char *current_pos = header_start;

	while(current_pos < body_start){
		const char *next_line = strstr(current_pos, "\r\n");
		if(!next_line || next_line == current_pos) break;

		header_count++;
		current_pos = next_line + 2;
	}

	req->header_count = header_count;
	req->headers = calloc(header_count, sizeof(*req->headers));

	current_pos = header_start;
	size_t index = 0;
	while(current_pos < body_start && index < header_count){
		const char *next_line = strstr(current_pos, "\r\n");
		if(!next_line) break;

		size_t line_length = next_line - current_pos;
		char *header_line = strndup(current_pos, line_length);

		char *colon_pos = strchr(header_line, ':');
		if (colon_pos) {
			*colon_pos = '\0';
			req->headers[index].key = strdup(header_line);

			char *raw_value = colon_pos + 1;
			while (*raw_value == ' ') raw_value++;  // skip spaces but keep valid pointer
			req->headers[index].value = strdup(raw_value);
		}



		free(header_line);
		index++;
		current_pos = next_line + 2;
	}

	body_start += 4; // Move past the "\r\n\r\n"
	if(*body_start){
		req->body = strdup(body_start);
	}
	else {
		req->body = NULL;
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


K_httpRes *k_http_res_create(){
	K_httpRes *res = calloc(1, sizeof(K_httpRes));
	if(!res) return NULL;

	res->status_code = 200;
	res->body = NULL;
	res->headers = NULL;
	res->header_count = 0;

	return res;
}


void k_http_res_set_body(K_httpRes *res, const char *body){
	if(!res) return;

	free(res->body);
	res->body = body ? strdup(body) : NULL;
}



void k_http_res_set_header(K_httpRes *res, const char *key, const char *value){
	if(!res || !key || !value) return;

	res->headers = realloc(res->headers, sizeof(*res->headers) * (res->header_count + 1));
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
	res->headers = NULL;
	res->header_count = 0;
}

int k_http_res_send(int client_socket, K_httpRes *res){
	if(!res) return -1;

	size_t buffer_size = 1024 + (res->body ? strlen(res->body) : 0);
	char *buffer = malloc(buffer_size);
	if(!buffer) return -1;

	int len = snprintf(buffer, buffer_size, "HTTP/1.1 %d %s\r\n",
						res->status_code,
						(res->status_code == 200) ? "OK" : "ERROR");

	for(size_t i = 0; i < res->header_count; i++){
		len += snprintf(buffer + len, buffer_size - len, "%s: %s\r\n",
						res->headers[i].key,
						res->headers[i].value);
	}

	len += snprintf(buffer + len, buffer_size - len, "\r\n%s",
					res->body ? res->body : "");

	ssize_t sent = send(client_socket, buffer, len, 0);
	free(buffer);
	return (sent == len) ? 0 : -1;
}
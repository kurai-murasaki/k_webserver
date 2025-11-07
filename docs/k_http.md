## k_http

`k_http` — HTTP request parsing and response helpers used by the core server.

### Overview

The `k_http` module provides small, dependency-free utilities to parse raw HTTP requests into a structured `K_httpReq`, build `K_httpRes` responses, and send responses over a socket with correct handling of partial writes. It's designed for minimal servers where you want simple, explicit ownership rules and easy composition with other server code.

The implementation focuses on correctness and clarity: it parses the request-line, headers, and body (when present), and offers convenience functions to create/set headers and body on responses. Memory ownership is explicit — the caller is responsible for freeing parsed requests and created responses.

### Features

- Parse HTTP request lines, headers, and body into `K_httpReq`.
- Build and manage HTTP responses (`K_httpRes`) with helpers to set body and headers.
- Send responses over a socket while handling partial `send()` calls.
- Small, self-contained API with no global state (easy to reason about and test).

### Usage

1. Include the header:

```c
#include "k_http.h"
```

2. Parse an incoming raw request (e.g. data read from a socket):

```c
K_httpReq *req = k_http_req_parse(buffer, buffer_len);
if (!req) {
    // parse or allocation failure
}

// Access request fields:
// req->method, req->uri, req->version, req->headers (array), req->header_count, req->body, req->body_length

// When finished:
k_http_req_free(req);
```

3. Create and send a response:

```c
K_httpRes *res = k_http_res_create(); // status defaults to 200
if (!res) { /* handle allocation failure */ }

k_http_res_set_header(res, "Content-Type", "text/plain");
const char *body = "Hello, world!";
k_http_res_set_body(res, body, strlen(body));

int rc = k_http_res_send(client_fd, res);
if (rc != 0) {
    // send error (socket closed, etc.)
}

k_http_res_free(res);
```

### API Reference

#### Types

- `K_httpHeader`
  - struct with `char *key;` and `char *value;` representing a single header field.

- `K_httpReq`
  - Fields:
    - `char *method` — HTTP method (e.g. "GET").
    - `char *uri` — Request URI (e.g. "/index.html").
    - `char *version` — HTTP version string (e.g. "HTTP/1.1").
    - `char *body` — Pointer to request body (NULL if none). The buffer is NUL-terminated.
    - `size_t body_length` — Length of the body in bytes.
    - `K_httpHeader *headers` — Array of parsed headers (may be NULL if none).
    - `size_t header_count` — Number of entries in `headers`.

- `K_httpRes`
  - Fields:
    - `int status_code` — HTTP status code (defaults to 200 from `k_http_res_create`).
    - `char *body` — Response body (NULL if none). The buffer is NUL-terminated.
    - `size_t body_length` — Length of the body in bytes.
    - `K_httpHeader *headers` — Array of response headers.
    - `size_t header_count` — Number of response headers.

#### Functions

- `K_httpReq *k_http_req_parse(const char *raw_request, size_t raw_req_len);`
  - Parse a raw HTTP request buffer.
  - Returns: pointer to a newly allocated `K_httpReq` on success — caller must call `k_http_req_free()`; returns `NULL` on invalid input or allocation failure.
  - Notes: The parser looks for the first CRLF to split the request line, then parses headers until the blank CRLFCRLF sequence. The function makes internal allocations for strings and header arrays.

- `void k_http_req_free(K_httpReq *req);`
  - Free all memory associated with `req`. Safe to call with `NULL`.

- `K_httpRes *k_http_res_create(void);`
  - Create a new response object. On allocation failure returns `NULL`.
  - The returned response has `status_code` set to 200 and no headers/body.

- `void k_http_res_set_body(K_httpRes *res, const char *body, size_t len);`
  - Set or replace the response body. Copies `len` bytes from `body` into an allocated buffer and NUL-terminates it. Passing `NULL` or `len == 0` clears the body.

- `void k_http_res_set_header(K_httpRes *res, const char *key, const char *value);`
  - Append a header to the response. Both `key` and `value` are duplicated.
  - If allocation fails, this function leaves the response unchanged.

- `void k_http_res_free(K_httpRes *res);`
  - Free all memory owned by the response. Safe to call with `NULL`.

- `int k_http_res_send(int client_socket, K_httpRes *res);`
  - Send the constructed response over `client_socket`. Handles partial `send()` calls internally.
  - Returns `0` on success, `-1` on failure (socket closed, `send()` error, or memory allocation failure while assembling the buffer).

#### Ownership and error semantics

- Functions that return allocated objects (`k_http_req_parse`, `k_http_res_create`) transfer ownership to the caller, who must free them with the corresponding `_free` function.
- Header and body setter functions duplicate inputs; callers retain ownership of their original buffers.
- `k_http_req_parse` may return a partially-populated `K_httpReq` in some malformed-input cases; check for `NULL` to detect total failure.

### Examples

Echo server snippet (simplified):

```c
// after reading `buf` from socket with length `n`
K_httpReq *req = k_http_req_parse(buf, n);
if (!req) { close(client_fd); return; }

K_httpRes *res = k_http_res_create();
if (!res) { k_http_req_free(req); close(client_fd); return; }

const char *body = "Hello from kurai-webserver!";
k_http_res_set_header(res, "Content-Type", "text/plain");
k_http_res_set_body(res, body, strlen(body));

if (k_http_res_send(client_fd, res) != 0) {
    // handle send error
}

k_http_res_free(res);
k_http_req_free(req);
```

### Notes

- The module intentionally makes few assumptions about framing: it parses a single HTTP request from a contiguous buffer. For streaming, incremental parsing, or HTTP/1.1 connection reuse, add a buffering layer above `k_http_req_parse`.
- There are no convenience status reason strings — only the numeric `status_code` is used when creating the status line.
- Keep socket handling (timeouts, non-blocking I/O) in server code; `k_http_res_send` performs blocking `send()` calls and returns an error on failure.


---
# 🖤 API 💜
---
## API
- [🖤] [k_err.h](API.md#k_err)
- [🖤] [k_http](API.md#k_http)
- [🖤] [k_json](API.md#k_json)
---

## k_err

`k_err` is a utility for error handling within the k_webserver project. It provides a unified way to represent, propagate, and interpret error codes throughout the API, making debugging and error management more consistent and reliable.

### Overview

The `k_err` module provides macros and helper functions to facilitate error reporting and handling. It does not define any error codes or constants by default—you must declare your own error codes using the provided macros. All declared errors must be documented, including their meaning and usage.


### Features

- **User-defined error codes:** No predefined constants; you must declare your own using macros.
- **Pointer-style error sentinels:** Special pointer values (e.g., (void*)-1) to signal errors from pointer-returning functions.
- **Simple API:** Minimal functions/macros for ease of use.
- **Documentation required:** Any error code you declare must be documented and explained.

### Usage

#### 1. Include the header

```c
#include "k_err.h"
```

#### 2. Declare and document your own errors

Use the provided macro to declare error codes. Document each error code and what it means.

```c
// Example: Json errors
#define JSON_PARSE_ERROR      1
#define JSON_INVALID_TYPE     2

K_DEFINE_ERR(JSON_PARSE_ERROR) // -1, pointer sentinel (void*)-1
K_DEFINE_ERR(JSON_INVALID_TYPE) // -2, pointer sentinel (void*)-2

// Document: NOT_FOUND means the requested resource does not exist.
```

#### 3. Return and check errors

For pointer-returning functions, use pointer-style error sentinels:

```c
K_jsonValue *json = k_json_parse(json_str, json_str_len);
if (k_ptr_is_err(json)) {
    int err = k_ptr_err_code(json);
    if (err == JSON_PARSE_ERROR) {
        // Handle JSON syntax error
    } else if (err == JSON_INVALID_TYPE) {
        // Handle invalid type error
    } else {
        // Handle unknown error
    }
}
```

For value-returning functions, use your own error codes or the K_err struct:

```c
K_err err = some_function();
if (err.code != 0) {
    printf("Error: %s\n", err.message);
    // Handle error
}
```
### API Reference

#### Types

- `K_err`  
  Struct containing an error code and message.

#### Macros

- `K_DEFINE_ERR(code)`  
  Declares an integer error code and a pointer-style error sentinel for `code`. You must document each error you declare.

#### Functions

- `int k_ptr_is_err(const void* ptr);`  
  Returns 1 if the pointer is an error sentinel, 0 otherwise.
- `int k_ptr_err_code(const void* ptr);`  
  Returns the error code from a pointer-style error sentinel.
- `K_err k_err_make(int code, const char* message);`  
  Creates a new error struct.
---


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


## k_json

`k_json` — small, in-repo JSON value representation and helpers (work in progress).

### Overview

The `k_json` module provides a compact value type (`K_jsonValue`) and a small set of helpers to parse, inspect and free JSON values used by the server. The module is intentionally minimal: it models JSON as a typed value with convenient accessors for boolean, integer, float and string values, plus array/object containers.

Important: the JSON parser and some convenience helpers are currently a work-in-progress in `core/k_json.c`. The API documented below reflects the header-level contract (`core/k_json.h`) — callers should treat parsing functions as not-yet-implemented until the C sources are completed.

### Features

- Typed JSON value representation (`K_jsonValue`) with enum discriminant `K_jsonType`.
- Accessor helpers for primitive values: `k_json_bool`, `k_json_int`, `k_json_float`, `k_json_string`.
- Object/array containers (arrays of `K_jsonValue *` and key/value arrays for objects).
- Memory management helper: `k_json_free` to recursively free a value.
- Error identifiers declared for consumers: `JSON_PARSE_ERROR`, `JSON_INVALID_TYPE` and pointer-sentinels via `K_DEFINE_ERR` (see `core/k_json.h`).

### Usage (high-level)

1. Include the header:

```c
#include "k_json.h"
```

2. Parse a JSON buffer (WIP — parser not implemented yet):

```c
K_jsonValue *val = k_json_parse(json_buf, json_len);
if (!val) {
    // parse failed or parser not implemented yet
}
// use accessors, then free
k_json_free(val);
```

3. Access primitive values using helpers:

```c
int b = k_json_bool(value);      // 1/0 or -1 on invalid
int i = k_json_int(value);       // integer value or (per header) NAN/invalid sentinel
double f = k_json_float(value);  // float or NAN on invalid
const char *s = k_json_string(value); // string pointer or NULL on invalid
```

### API Reference

#### Types

- `K_jsonType` (enum)
  - `K_JSON_NULL`
  - `K_JSON_BOOL`
  - `K_JSON_INT`
  - `K_JSON_FLOAT`
  - `K_JSON_STRING`
  - `K_JSON_ARRAY`
  - `K_JSON_OBJECT`

- `K_jsonValue` (struct)
  - `K_jsonType type` — discriminant.
  - Union members depending on type:
    - `int bool_value`
    - `int int_value`
    - `float float_value`
    - `char *string_value` (heap owned, freed by `k_json_free`)
    - `array` struct: `K_jsonValue **items; size_t count;`
    - `object` struct: `char **keys; K_jsonValue **values; size_t count;`

#### Macros / Errors

- `JSON_PARSE_ERROR` — numeric identifier; `K_DEFINE_ERR(JSON_PARSE_ERROR)` is used in the header to provide a pointer-style sentinel.
- `JSON_INVALID_TYPE` — numeric identifier for invalid-type errors; also wrapped by `K_DEFINE_ERR` in the header.

Note: The header defines these macros and then invokes `K_DEFINE_ERR(...)`. The header also undefines the numeric macros at the end to avoid leaking them as preprocessor symbols.

#### Functions

- `K_jsonValue* k_json_parse(const char *json_str, size_t json_str_len);`
  - Parse the input JSON buffer and return a newly-allocated `K_jsonValue*` on success. The header documents that this returns `NULL` on failure. Implementation: currently a stub in `core/k_json.c` (WIP).

- `char* k_json_serialize(K_jsonValue* value);`
  - Serialize `value` to a newly-allocated string (caller frees). Declaration present in header; implementation may be added later.

- `K_jsonValue* k_json_get_object(K_jsonValue* obj, const char* key);`
  - Lookup a key in an object-valued `K_jsonValue`. Declared in the header; implementation currently a stub in `core/k_json.c` (WIP).

- `void k_json_free(K_jsonValue* value);`
  - Recursively free a JSON value and its contents. Safe to call with `NULL`.

- `int k_json_bool(K_jsonValue* value);`
  - Return boolean value (1 for true, 0 for false) or -1 for invalid input (per header documentation).

- `double k_json_float(K_jsonValue* value);`
  - Return float value, or `NAN` for invalid input.

- `int k_json_int(K_jsonValue* value);`
  - Return integer value; header documentation notes `NAN` for invalid, but the function returns an `int` and should be treated as returning an implementation-defined sentinel on invalid input — check the header/source for exact sentinel semantics.

- `const char* k_json_string(K_jsonValue* value);`
  - Return pointer to the string value or `NULL` for invalid input.

### Ownership and error semantics

- Functions that return allocated objects (`k_json_parse`) transfer ownership to the caller; callers must free with `k_json_free`.
- `k_json_free` recursively frees object and array contents and their string/keys; it is safe to call with `NULL`.
- Accessor functions return simple values or sentinels on invalid input; callers should check return values per the header's documented sentinels.

### Status / Notes

- The JSON module is explicitly marked WIP in the sources — `k_json_parse` and `k_json_get_object` are currently unimplemented stubs in `core/k_json.c`. This documentation intentionally sticks to the header's declarations and does not guess parser behavior beyond what the header states.
- When the parser and serializer are implemented, consider adding small unit tests exercising parsing, serialization round-trips, and object lookups.

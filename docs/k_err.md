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
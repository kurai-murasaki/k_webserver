/**
* @file k_err.h
* @brief Error handling utilities for the k_webserver core.
*
* @details
* This header provides a unified and lightweight error handling mechanism for C/C++ projects,
* especially suitable for systems programming or server backends where performance and clarity are critical.
*
* The utility defines:
* - A simple error struct (`K_err`) for returning error codes and messages.
* - A set of universal error codes (negative integers) for common error conditions.
* - Macros to define custom error codes, reserving certain values for internal use.
* - "Pointer-style" error sentinels: special pointer values (e.g., (void*)-1) that encode error states,
*   allowing functions that return pointers to signal errors without needing out-of-band mechanisms.
* - Helper inline functions to check for pointer errors and extract error codes.
*
* Usage patterns:
* - For functions returning structs or values, return a `K_err` or an error code.
* - For functions returning pointers, use the pointer error sentinels and check with `k_ptr_is_err()`.
* - Custom error codes can be defined using the `K_ERR_(code)` and `K_PTR_ERR_(code)` macros.
*
* This approach is inspired by Linux kernel error handling, providing fast, branchless error checks
* and minimizing heap allocations for error reporting.
*
* Example:
*   void* resource = get_resource();
*   if (k_ptr_is_err(resource)) {
*       int err = k_ptr_err_code(resource);
*       // handle error
*   }
*
* @author Kurai
* @date 2025-11-06
* @copyright Copyright (c) 2025 Kurai Murasaki
* @license NotForSaleNotForYou - See LICENSE file for details
*/

#ifndef K_ERR_H
#define K_ERR_H


/**
* Include files organization:
* 1. Standard C library headers
* 2. External third-party dependencies
* 3. Internal library headers
*/


// Standard library includes
#include <stddef.h>
#include <stdint.h>

// External dependencies


// Internal library includes


typedef struct K_err{
    int code;
    const char* message;
} K_err;


// === Fast pointer-style error sentinels ===
// These are used when function returns a pointer instead of an error struct.
// Never dereference these. Always compare directly.

#define K_PTR_ERR_(code) ((void*)(intptr_t)(code))


// === Error helpers ===

/**
* @name k_ptr_is_err()
* @brief Check if a pointer is an error sentinel.
* @param ptr Pointer to check.
* @return 1 if the pointer is an error sentinel, 0 otherwise.
*/
static inline int k_ptr_is_err(const void* ptr){
    return (int)((intptr_t)ptr >= (intptr_t)-4095);
}

/**
* @name k_ptr_err_code()
* @brief Get the error code from a pointer.
* @param ptr Pointer to check.
* @return Error code or K_OK if not an error pointer.
*/
static inline int k_ptr_err_code(const void* ptr){
    return (int)(intptr_t)ptr;
}

/**
* @name k_err_make()
* @brief Create a new error.
* @param code Error code.
* @param message Error message.
* @return New error.
*/
static inline K_err k_err_make(int code, const char* message){
    K_err err;
    err.code = code;
    err.message = message;
    return err;
}


//=== macros ===
#define K_DEFINE_ERR(code)           \
    static const int K_ERR_##code = -(code); \
    static const void* K_PTR_ERR_##code = ((void*)(intptr_t)(-(code)));


#endif // K_ERR_H
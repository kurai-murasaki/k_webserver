/**
* @file k_json.h
* @brief 
* @author Kurai
* @date 2025-11-06
*
* @copyright Copyright (c) 2025 Kurai Murasaki
* @license NotForSaleNotForYou - See LICENSE file for details
*
* @details 
*/

#ifndef K_JSON_H
#define K_JSON_H


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
#include <core/k_err.h>

// Define error identifiers for JSON errors
#define JSON_PARSE_ERROR      1
#define JSON_INVALID_TYPE     2

K_DEFINE_ERR(JSON_PARSE_ERROR)
K_DEFINE_ERR(JSON_INVALID_TYPE)


/**
* @enum K_jsonType
* @brief JSON value types
* @value K_JSON_NULL Represents a null value.
* @value K_JSON_BOOL Represents a boolean value.
* @value K_JSON_NUMBER Represents a numeric value.
* @value K_JSON_STRING Represents a string value.
* @value K_JSON_ARRAY Represents an array value.
* @value K_JSON_OBJECT Represents an object value.
* @details This enum defines the various types of JSON values.
*/
typedef enum {
    K_JSON_NULL,
    K_JSON_BOOL,
    K_JSON_INT,
    K_JSON_FLOAT,
    K_JSON_STRING,
    K_JSON_ARRAY,
    K_JSON_OBJECT
} K_jsonType;


/**
* @struct K_jsonValue
* @brief JSON value structure
* @details This structure holds the details of a JSON value.
* @param type Type of the JSON value (e.g., null, boolean, number, string, array, object)
* @param bool_value Boolean value (if type is K_JSON_BOOL)
* @param int_value Integer value (if type is K_JSON_INT)
* @param float_value Float value (if type is K_JSON_FLOAT)
* @param string_value String value (if type is K_JSON_STRING)
* @param array_value Array value (if type is K_JSON_ARRAY)
* @param object_value Object value (if type is K_JSON_OBJECT)
*/
typedef struct {
    K_jsonType type;
    union {
        int bool_value;
        int int_value;
        float float_value;
        char *string_value;
        struct {
            struct K_jsonValue **items;
            size_t count;
        } array;
        struct {
            char **keys;
            struct K_jsonValue **values;
            size_t count;
        } object;
    };

} K_jsonValue;

 
// === JSON functions ===

/**
* @name k_json_parse()
* @brief Parse a JSON string.
* @param json_str Pointer to the JSON string.
* @return Pointer to the parsed JSON value structure. Nullptr on failure.
*/
K_jsonValue* k_json_parse(const char *json_str, size_t json_str_len);


/**
* @name k_json_serialize()
* @brief Serialize a JSON value to a string.
* @param value Pointer to the JSON value.
* @return Pointer to the serialized JSON string. Nullptr on failure.
*/
char* k_json_serialize(K_jsonValue* value);


/**
* @name k_json_get_object()
* @brief Get a JSON object by key.
* @param obj Pointer to the JSON object.
* @param key Pointer to the key string.
* @return Pointer to the JSON value associated with the key. Nullptr if key not found.
*/
K_jsonValue* k_json_get_object(K_jsonValue* obj, const char* key);


/**
* @name k_json_free()
* @brief Free a JSON value.
* @param value Pointer to the JSON value to free. 
*/
void k_json_free(K_jsonValue* value);

/**
* @name k_json_bool()
* @brief Get a JSON boolean value.
* @param value Pointer to the JSON value.
* @return Boolean value (1 for true, 0 for false) -1 for invalid.
*/
int k_json_bool(K_jsonValue* value);

/**
* @name k_json_float()
* @brief Get a JSON float value.
* @param value Pointer to the JSON value.
* @return Float value. NAN for invalid.
*/
double k_json_float(K_jsonValue* value);

/**
* @name k_json_int()
* @brief Get a JSON integer value.
* @param value Pointer to the JSON value.
* @return Int value. NAN for invalid.
*/
int k_json_int(K_jsonValue* value);


/**
* @name k_json_string()
* @brief Get a JSON string value.
* @param value Pointer to the JSON value.
* @return Pointer to the string value. NULL for invalid.
*/
const char* k_json_string(K_jsonValue* value);


//Later create own array type, and use it here.
// size_t k_json_array_size(K_jsonValue* value);
// K_jsonValue* k_json_array_get(K_jsonValue* value, size_t index);



#undef JSON_PARSE_ERROR
#undef JSON_INVALID_TYPE

#endif // K_JSON_H
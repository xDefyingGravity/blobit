//
// Created by Will Ballantine on 10/8/25.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#define BLOBIT_SUCCESS 0 // Success
#define BLOBIT_FAILURE 1 // Failure

/* Basic types */

typedef int64_t blobit_int; // 64-bit integer
typedef double blobit_float; // 64-bit floating point
typedef bool blobit_bool; // Boolean
typedef char *blobit_string; // String

/**
 * Get a human-readable error message for a given error code.
 * @param code - The error code.
 * @return - A string describing the error.
 */
const char* blobit_error_message(int code);

/**
 * Assert that a blobit function call succeeds, otherwise print an error message and return the error code.
 * @param func - Function to execute that returns a blobit error code.
 * @param fail_message - Message to print on failure.
 * @param fail_stream - Stream to print the failure message to (e.g., stderr).
 * @return - The error code from the function if it fails, otherwise continues execution
 */
#define blobit_assert(func, fail_message, fail_stream) \
    do { \
        int res = (func); \
        if (res != BLOBIT_SUCCESS) { \
            fprintf(fail_stream, "Error (%s): %s\n", blobit_error_message(res), fail_message); \
            return res; \
        } \
    } while (0)

/**
 * Blobit error codes.
 */
typedef enum {
    BLOBIT_ERROR_INVALID_TYPE = -1, // Invalid type
    BLOBIT_ERROR_MAP_KEY_NOT_FOUND = -2, // Map key not found
    BLOBIT_ERROR_ARRAY_INDEX_OUT_OF_BOUNDS = -3, // Array index out of bounds
    BLOBIT_ERROR_NULL_POINTER = -4, // Null pointer argument
    BLOBIT_ERROR_ALLOCATION_FAILED = -5, // Memory allocation failed
} blobit_error_t;

/**
 * Default serializer identifier for blobit (can be used in blobit_serializer_t).
 */
#define blobit_default NULL

/**
 * Blobit value types.
 */
enum blobit_value_type_t {
    BLOB_TYPE_NIL, // Null type
    BLOB_TYPE_BOOL, // Boolean type
    BLOB_TYPE_INT, // Integer type
    BLOB_TYPE_FLOAT, // Float type
    BLOB_TYPE_STRING, // String type
    BLOB_TYPE_ARRAY, // Array type
    BLOB_TYPE_MAP, // Map type
};

/**
 * Blobit value structure.
 * Represents a dynamically typed value that can hold various types including int, float, bool, string, array, and map.
 * The structure uses a union to store the actual value based on the type.
 */
struct blobit_value_t {
    enum blobit_value_type_t type;
    union {
        blobit_int int_val;
        blobit_float float_val;
        blobit_bool bool_val;
        blobit_string string_val;
        struct {
            struct blobit_value_t **items;
            size_t length;
            size_t capacity;
        } array_val;
        struct {
            struct blobit_value_t **keys;
            struct blobit_value_t **values;
            size_t length;
            size_t capacity;
        } map_val;
    };
};

typedef struct blobit_value_t blobit_value_t;
typedef enum blobit_value_type_t blobit_value_type_t;

/**
 * Create a new blobit value of the specified type.
 * @param val - Pointer to store the allocated blobit_value_t*.
 * @param type - The type of value to create (see blobit_value_type_t).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_create(blobit_value_t** val, blobit_value_type_t type);

/**
 * Destroy a blobit value and all its children (recursively for arrays/maps).
 * @param val - The value to destroy.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_destroy(blobit_value_t* val);

/**
 * Assign an integer value to a blobit value of type INT.
 * @param val - The value to assign to (must be type INT).
 * @param int_val - The integer value to assign.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_assign_int(blobit_value_t* val, blobit_int int_val);

/**
 * Assign a float value to a blobit value of type FLOAT.
 * @param val - The value to assign to (must be type FLOAT).
 * @param float_val - The float value to assign.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_assign_float(blobit_value_t *val, blobit_float float_val);

/**
 * Assign a boolean value to a blobit value of type BOOL.
 * @param val - The value to assign to (must be type BOOL).
 * @param bool_val - The boolean value to assign.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_assign_bool(blobit_value_t *val, blobit_bool bool_val);

/**
 * Assign a string value to a blobit value of type STRING.
 * The string is copied internally.
 * @param val - The value to assign to (must be type STRING).
 * @param string_val - The string to assign (UTF-8, null-terminated).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_assign_string(blobit_value_t *val, blobit_string string_val);

/**
 * Get the integer value from a blobit value of type INT.
 * @param val - The value to read from (must be type INT).
 * @param out_int - Pointer to store the integer value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_get_int(const blobit_value_t *val, blobit_int *out_int);

/**
 * Get the float value from a blobit value of type FLOAT.
 * @param val - The value to read from (must be type FLOAT).
 * @param out_float - Pointer to store the float value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_get_float(const blobit_value_t *val, blobit_float *out_float);

/**
 * Get the boolean value from a blobit value of type BOOL.
 * @param val - The value to read from (must be type BOOL).
 * @param out_bool - Pointer to store the boolean value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_get_bool(const blobit_value_t *val, blobit_bool *out_bool);

/**
 * Get the string value from a blobit value of type STRING.
 * @param val - The value to read from (must be type STRING).
 * @param out_string - Pointer to store the string (do not free, owned by blobit).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_get_string(const blobit_value_t *val, blobit_string *out_string);

/**
 * Print a blobit value to a stream in a human-readable format.
 * @param val - The value to print.
 * @param stream - The output stream (e.g. stdout).
 */
void blobit_value_print(const blobit_value_t *val, FILE *stream);

/**
 * Append an item to a blobit array value.
 * @param arr - The array value (must be type ARRAY).
 * @param item - The item to append (ownership is transferred).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_array_append(blobit_value_t *arr, blobit_value_t *item);

/**
 * Insert a key-value pair into a blobit map value.
 * @param map - The map value (must be type MAP).
 * @param key - The key (ownership is transferred).
 * @param value - The value (ownership is transferred).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_map_insert(blobit_value_t *map, blobit_value_t *key, blobit_value_t *value);

/**
 * Get an item from a blobit array by index.
 * @param arr - The array value (must be type ARRAY).
 * @param index - The index of the item.
 * @param out_item - Pointer to store the item.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_array_get(const blobit_value_t *arr, size_t index, blobit_value_t **out_item);

/**
 * Get a value from a blobit map by key pointer (pointer equality).
 * @param map - The map value (must be type MAP).
 * @param key - The key (pointer equality).
 * @param out_value - Pointer to store the value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_map_get(const blobit_value_t *map, blobit_value_t *key, blobit_value_t **out_value);

/**
 * Remove an item from a blobit array by index.
 * @param arr - The array value (must be type ARRAY).
 * @param index - The index of the item to remove.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_array_remove(blobit_value_t *arr, size_t index);

/**
 * Remove a key-value pair from a blobit map by key pointer (pointer equality).
 * @param map - The map value (must be type MAP).
 * @param key - The key (pointer equality).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_map_remove(blobit_value_t *map, blobit_value_t *key);

/**
 * Get the length of a blobit array.
 * @param arr - The array value (must be type ARRAY).
 * @param out_length - Pointer to store the length.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_array_length(const blobit_value_t *arr, size_t *out_length);

/**
 * Get the number of key-value pairs in a blobit map.
 * @param map - The map value (must be type MAP).
 * @param out_length - Pointer to store the length.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_map_length(const blobit_value_t *map, size_t *out_length);

/**
 * Remove all items from a blobit array and free their memory.
 * @param arr - The array value (must be type ARRAY).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_array_clear(blobit_value_t *arr);

/**
 * Remove all key-value pairs from a blobit map and free their memory.
 * @param map - The map value (must be type MAP).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_map_clear(blobit_value_t *map);

/**
 * Deep copy a blobit value (recursively copies arrays/maps/strings).
 * @param val - The value to copy.
 * @param out_val - Pointer to store the new copy.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_value_copy(const blobit_value_t *val, blobit_value_t **out_val);

/**
 * Blobit serializer interface. Allows custom serialization/deserialization.
 * Set to blobit_default for the built-in format.
 */
typedef struct blobit_serializer_t {
    int (*serialize)(const blobit_value_t* val, void* user_data, uint8_t** out_buf, size_t* out_size);
    int (*deserialize)(const uint8_t* buf, size_t size, void* user_data, blobit_value_t** out_val);
    void* user_data;
} blobit_serializer_t;

/**
 * Serialize a value using a custom serializer (or default if NULL).
 * @param val - The value to serialize.
 * @param serializer - The serializer to use (or blobit_default).
 * @param out_buf - Pointer to store the allocated output buffer.
 * @param out_size - Pointer to store the output buffer size.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_serialize_with(const blobit_value_t* val, const blobit_serializer_t* serializer, uint8_t** out_buf, size_t* out_size);

/**
 * Deserialize a value using a custom serializer (or default if NULL).
 * @param buf - The input buffer.
 * @param size - The size of the input buffer.
 * @param serializer - The serializer to use (or blobit_default).
 * @param out_val - Pointer to store the deserialized value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_deserialize_with(const uint8_t* buf, size_t size, const blobit_serializer_t* serializer, blobit_value_t** out_val);

/**
 * Serialize a value using the default serializer.
 * @param val - The value to serialize.
 * @param out_buf - Pointer to store the allocated output buffer.
 * @param out_size - Pointer to store the output buffer size.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_serialize(const blobit_value_t* val, uint8_t** out_buf, size_t* out_size);

/**
 * Deserialize a value using the default serializer.
 * @param buf - The input buffer.
 * @param size - The size of the input buffer.
 * @param out_val - Pointer to store the deserialized value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_deserialize(const uint8_t* buf, size_t size, blobit_value_t** out_val);

/**
 * Serialize a value to a file using a custom serializer (or default if NULL).
 * @param val - The value to serialize.
 * @param filename - The file to write to.
 * @param serializer - The serializer to use (or blobit_default).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_serialize_to_file_with(const blobit_value_t* val, const char* filename, const blobit_serializer_t* serializer);

/**
 * Deserialize a value from a file using a custom serializer (or default if NULL).
 * @param filename - The file to read from.
 * @param out_val - Pointer to store the deserialized value.
 * @param serializer - The serializer to use (or blobit_default).
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_deserialize_from_file_with(const char* filename, blobit_value_t** out_val, const blobit_serializer_t* serializer);

/**
 * Serialize a value to a file using the default serializer.
 * @param val - The value to serialize.
 * @param filename - The file to write to.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_serialize_to_file(const blobit_value_t* val, const char* filename);

/**
 * Deserialize a value from a file using the default serializer.
 * @param filename - The file to read from.
 * @param out_val - Pointer to store the deserialized value.
 * @return - BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_deserialize_from_file(const char* filename, blobit_value_t** out_val);

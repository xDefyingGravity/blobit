//
// Created by Will Ballantine on 10/8/25.

#pragma once
#include <blobit/blobit.h>

/**
 * blobit default serializer.
 *
 * @param val The value to serialize.
 * @param user_data Optional user data pointer (can be NULL).
 * @param out_buf Pointer to store the allocated output buffer.
 * @param out_size Pointer to store the output buffer size.
 * @return BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_default_serialize(const blobit_value_t* val, void* user_data, uint8_t** out_buf, size_t* out_size);

/**
 * blobit default deserializer.
 *
 * @param buf The input buffer to deserialize.
 * @param size Size of the input buffer in bytes.
 * @param user_data Optional user data pointer (can be NULL).
 * @param out_val Pointer to store the allocated blobit_value_t*.
 * @return BLOBIT_SUCCESS on success, otherwise an error code.
 */
int blobit_default_deserialize(const uint8_t* buf, size_t size, void* user_data, blobit_value_t** out_val);
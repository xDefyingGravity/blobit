//
// Created by Will Ballantine on 10/8/25.
//

#include <sys/errno.h>
#include "blobit/blobit.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "blobit/__private/default_serializer.h"

int blobit_value_create(blobit_value_t** val, blobit_value_type_t type)
{
    if (!val)
        return BLOBIT_ERROR_NULL_POINTER;
    *val = (blobit_value_t*)malloc(sizeof(blobit_value_t));
    if (!*val)
        return BLOBIT_ERROR_ALLOCATION_FAILED;
    (*val)->type = type;
    switch (type)
    {
    case BLOB_TYPE_ARRAY:
        (*val)->array_val.items = NULL;
        (*val)->array_val.length = 0;
        (*val)->array_val.capacity = 0;
        break;
    case BLOB_TYPE_MAP:
        (*val)->map_val.keys = NULL;
        (*val)->map_val.values = NULL;
        (*val)->map_val.length = 0;
        (*val)->map_val.capacity = 0;
        break;
    case BLOB_TYPE_STRING:
        (*val)->string_val = NULL;
        break;
    default:
        break;
    }
    return BLOBIT_SUCCESS;
}

int blobit_value_assign_int(blobit_value_t* val, blobit_int int_val)
{
    if (!val)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_INT)
        return BLOBIT_ERROR_INVALID_TYPE;
    val->int_val = int_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_assign_float(blobit_value_t* val, blobit_float float_val)
{
    if (!val)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_FLOAT)
        return BLOBIT_ERROR_INVALID_TYPE;
    val->float_val = float_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_assign_bool(blobit_value_t* val, blobit_bool bool_val)
{
    if (!val)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_BOOL)
        return BLOBIT_ERROR_INVALID_TYPE;
    val->bool_val = bool_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_assign_string(blobit_value_t* val, blobit_string string_val)
{
    if (!val)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_STRING)
        return BLOBIT_ERROR_INVALID_TYPE;
    if (!string_val)
        return BLOBIT_ERROR_NULL_POINTER;
    size_t n = strlen(string_val);
    char* new_mem = malloc(n + 1);
    if (!new_mem)
        return BLOBIT_ERROR_ALLOCATION_FAILED;
    memcpy(new_mem, string_val, n + 1);
    if (val->string_val)
        free(val->string_val);
    val->string_val = new_mem;
    return BLOBIT_SUCCESS;
}


int blobit_value_get_int(const blobit_value_t* val, blobit_int* out_int)
{
    if (!val || !out_int)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_INT)
        return BLOBIT_ERROR_INVALID_TYPE;
    *out_int = val->int_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_get_float(const blobit_value_t* val, blobit_float* out_float)
{
    if (!val || !out_float)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_FLOAT)
        return BLOBIT_ERROR_INVALID_TYPE;
    *out_float = val->float_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_get_bool(const blobit_value_t* val, blobit_bool* out_bool)
{
    if (!val || !out_bool)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_BOOL)
        return BLOBIT_ERROR_INVALID_TYPE;
    *out_bool = val->bool_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_get_string(const blobit_value_t* val, blobit_string* out_string)
{
    if (!val || !out_string)
        return BLOBIT_ERROR_NULL_POINTER;
    if (val->type != BLOB_TYPE_STRING)
        return BLOBIT_ERROR_INVALID_TYPE;
    *out_string = val->string_val;
    return BLOBIT_SUCCESS;
}

int blobit_value_destroy(blobit_value_t* val)
{
    if (!val)
        return BLOBIT_ERROR_NULL_POINTER;

    switch (val->type)
    {
    case BLOB_TYPE_STRING:
        free(val->string_val);
        break;

    case BLOB_TYPE_ARRAY:
        if (val->array_val.items)
        {
            for (size_t i = 0; i < val->array_val.length; i++)
            {
                if (val->array_val.items[i])
                    blobit_value_destroy(val->array_val.items[i]);
            }
            free(val->array_val.items);
        }
        break;

    case BLOB_TYPE_MAP:
        if (val->map_val.keys && val->map_val.values)
        {
            for (size_t i = 0; i < val->map_val.length; i++)
            {
                if (val->map_val.keys[i])
                    blobit_value_destroy(val->map_val.keys[i]);
                if (val->map_val.values[i])
                    blobit_value_destroy(val->map_val.values[i]);
            }
            free(val->map_val.keys);
            free(val->map_val.values);
        }
        break;

    default:
        break;
    }

    free(val);
    return BLOBIT_SUCCESS;
}

static void blobit_value_print_recursive(const blobit_value_t* val, FILE* stream, int indent)
{
    if (!val)
    {
        fprintf(stream, "nil");
        return;
    }

    switch (val->type)
    {
    case BLOB_TYPE_INT:
        fprintf(stream, "%" PRId64, val->int_val);
        break;

    case BLOB_TYPE_FLOAT:
        fprintf(stream, "%f", val->float_val);
        break;

    case BLOB_TYPE_BOOL:
        fprintf(stream, val->bool_val ? "true" : "false");
        break;

    case BLOB_TYPE_STRING:
        fprintf(stream, "\"%s\"", val->string_val);
        break;

    case BLOB_TYPE_ARRAY:
        fprintf(stream, "[");
        for (size_t i = 0; i < val->array_val.length; i++)
        {
            if (i > 0) fprintf(stream, ", ");
            blobit_value_print_recursive(val->array_val.items[i], stream, indent + 1);
        }
        fprintf(stream, "]");
        break;

    case BLOB_TYPE_MAP:
        fprintf(stream, "{");
        for (size_t i = 0; i < val->map_val.length; i++)
        {
            if (i > 0) fprintf(stream, ", ");
            blobit_value_print_recursive(val->map_val.keys[i], stream, indent + 1);
            fprintf(stream, ": ");
            blobit_value_print_recursive(val->map_val.values[i], stream, indent + 1);
        }
        fprintf(stream, "}");
        break;

    default:
        fprintf(stream, "nil");
        break;
    }
}

void blobit_value_print(const blobit_value_t* val, FILE* stream)
{
    blobit_value_print_recursive(val, stream, 0);
}

int blobit_array_append(blobit_value_t* arr, blobit_value_t* item)
{
    if (!arr || !item)
        return BLOBIT_ERROR_NULL_POINTER;
    if (arr->type != BLOB_TYPE_ARRAY)
        return BLOBIT_ERROR_INVALID_TYPE;
    if (arr->array_val.length >= arr->array_val.capacity)
    {
        size_t new_cap = arr->array_val.capacity ? arr->array_val.capacity * 2 : 4;
        blobit_value_t** new_items = realloc(arr->array_val.items, new_cap * sizeof(blobit_value_t*));
        if (!new_items)
            return BLOBIT_ERROR_ALLOCATION_FAILED;
        arr->array_val.items = new_items;
        arr->array_val.capacity = new_cap;
    }
    arr->array_val.items[arr->array_val.length++] = item;
    return BLOBIT_SUCCESS;
}

int blobit_map_insert(blobit_value_t* map, blobit_value_t* key, blobit_value_t* value)
{
    if (!map || !key || !value)
        return BLOBIT_ERROR_NULL_POINTER;
    if (map->type != BLOB_TYPE_MAP)
        return BLOBIT_ERROR_INVALID_TYPE;
    if (map->map_val.length >= map->map_val.capacity)
    {
        size_t new_cap = map->map_val.capacity ? map->map_val.capacity * 2 : 4;
        size_t bytes = new_cap * sizeof(blobit_value_t*);
        blobit_value_t** new_keys = malloc(bytes);
        if (!new_keys)
            return BLOBIT_ERROR_ALLOCATION_FAILED;
        blobit_value_t** new_values = malloc(bytes);
        if (!new_values)
        {
            free(new_keys);
            return BLOBIT_ERROR_ALLOCATION_FAILED;
        }
        if (map->map_val.keys)
            memcpy(new_keys, map->map_val.keys, map->map_val.length * sizeof(blobit_value_t*));
        if (map->map_val.values)
            memcpy(new_values, map->map_val.values, map->map_val.length * sizeof(blobit_value_t*));
        free(map->map_val.keys);
        free(map->map_val.values);
        map->map_val.keys = new_keys;
        map->map_val.values = new_values;
        map->map_val.capacity = new_cap;
    }
    size_t idx = map->map_val.length++;
    map->map_val.keys[idx] = key;
    map->map_val.values[idx] = value;
    return BLOBIT_SUCCESS;
}

int blobit_map_get(const blobit_value_t* map, blobit_value_t* key, blobit_value_t** out_value)
{
    if (!map || !key || !out_value)
        return BLOBIT_ERROR_NULL_POINTER;
    if (map->type != BLOB_TYPE_MAP)
        return BLOBIT_ERROR_INVALID_TYPE;
    for (size_t i = 0; i < map->map_val.length; i++)
    {
        if (map->map_val.keys[i] == key)
        {
            *out_value = map->map_val.values[i];
            return BLOBIT_SUCCESS;
        }
    }
    return BLOBIT_ERROR_MAP_KEY_NOT_FOUND;
}

int blobit_array_get(const blobit_value_t* arr, size_t index, blobit_value_t** out_item)
{
    if (!arr || !out_item)
        return BLOBIT_ERROR_NULL_POINTER;
    if (arr->type != BLOB_TYPE_ARRAY)
        return BLOBIT_ERROR_INVALID_TYPE;
    if (index >= arr->array_val.length)
        return BLOBIT_ERROR_ARRAY_INDEX_OUT_OF_BOUNDS;
    *out_item = arr->array_val.items[index];
    return BLOBIT_SUCCESS;
}

int blobit_array_remove(blobit_value_t* arr, size_t index)
{
    if (!arr)
        return BLOBIT_ERROR_NULL_POINTER;
    if (arr->type != BLOB_TYPE_ARRAY)
        return BLOBIT_ERROR_INVALID_TYPE;
    if (index >= arr->array_val.length)
        return BLOBIT_ERROR_ARRAY_INDEX_OUT_OF_BOUNDS;
    blobit_value_destroy(arr->array_val.items[index]);
    for (size_t i = index + 1; i < arr->array_val.length; i++)
        arr->array_val.items[i - 1] = arr->array_val.items[i];
    arr->array_val.length--;
    return BLOBIT_SUCCESS;
}

int blobit_map_remove(blobit_value_t* map, blobit_value_t* key)
{
    if (!map || !key)
        return BLOBIT_ERROR_NULL_POINTER;
    if (map->type != BLOB_TYPE_MAP)
        return BLOBIT_ERROR_INVALID_TYPE;
    for (size_t i = 0; i < map->map_val.length; i++)
    {
        if (map->map_val.keys[i] == key)
        {
            blobit_value_destroy(map->map_val.keys[i]);
            blobit_value_destroy(map->map_val.values[i]);
            for (size_t j = i + 1; j < map->map_val.length; j++)
            {
                map->map_val.keys[j - 1] = map->map_val.keys[j];
                map->map_val.values[j - 1] = map->map_val.values[j];
            }
            map->map_val.length--;
            return BLOBIT_SUCCESS;
        }
    }
    return BLOBIT_ERROR_MAP_KEY_NOT_FOUND;
}

int blobit_array_length(const blobit_value_t* arr, size_t* out_length)
{
    if (!arr || !out_length)
        return BLOBIT_ERROR_NULL_POINTER;
    if (arr->type != BLOB_TYPE_ARRAY)
        return BLOBIT_ERROR_INVALID_TYPE;
    *out_length = arr->array_val.length;
    return BLOBIT_SUCCESS;
}

int blobit_map_length(const blobit_value_t* map, size_t* out_length)
{
    if (!map || !out_length)
        return BLOBIT_ERROR_NULL_POINTER;
    if (map->type != BLOB_TYPE_MAP)
        return BLOBIT_ERROR_INVALID_TYPE;
    *out_length = map->map_val.length;
    return BLOBIT_SUCCESS;
}

int blobit_array_clear(blobit_value_t* arr)
{
    if (!arr)
        return BLOBIT_ERROR_NULL_POINTER;
    if (arr->type != BLOB_TYPE_ARRAY)
        return BLOBIT_ERROR_INVALID_TYPE;
    for (size_t i = 0; i < arr->array_val.length; i++)
        blobit_value_destroy(arr->array_val.items[i]);
    free(arr->array_val.items);
    arr->array_val.items = NULL;
    arr->array_val.length = 0;
    arr->array_val.capacity = 0;
    return BLOBIT_SUCCESS;
}

int blobit_map_clear(blobit_value_t* map)
{
    if (!map)
        return BLOBIT_ERROR_NULL_POINTER;
    if (map->type != BLOB_TYPE_MAP)
        return BLOBIT_ERROR_INVALID_TYPE;
    for (size_t i = 0; i < map->map_val.length; i++)
    {
        blobit_value_destroy(map->map_val.keys[i]);
        blobit_value_destroy(map->map_val.values[i]);
    }
    free(map->map_val.keys);
    free(map->map_val.values);
    map->map_val.keys = NULL;
    map->map_val.values = NULL;
    map->map_val.length = 0;
    map->map_val.capacity = 0;
    return BLOBIT_SUCCESS;
}

int blobit_value_copy(const blobit_value_t* val, blobit_value_t** out_val)
{
    if (!val || !out_val)
        return BLOBIT_ERROR_NULL_POINTER;

    blobit_value_t* new_val = malloc(sizeof(blobit_value_t));
    if (!new_val)
        return BLOBIT_ERROR_ALLOCATION_FAILED;

    new_val->type = val->type;

    switch (val->type)
    {
    case BLOB_TYPE_INT:
        new_val->int_val = val->int_val;
        break;
    case BLOB_TYPE_FLOAT:
        new_val->float_val = val->float_val;
        break;
    case BLOB_TYPE_BOOL:
        new_val->bool_val = val->bool_val;
        break;
    case BLOB_TYPE_STRING:
        new_val->string_val = strdup(val->string_val);
        if (!new_val->string_val)
        {
            free(new_val);
            return BLOBIT_ERROR_ALLOCATION_FAILED;
        }
        break;
    case BLOB_TYPE_ARRAY:
        new_val->array_val.length = val->array_val.length;
        new_val->array_val.items = calloc(val->array_val.length, sizeof(blobit_value_t*));
        if (!new_val->array_val.items)
        {
            free(new_val);
            return BLOBIT_ERROR_ALLOCATION_FAILED;
        }
        for (size_t i = 0; i < val->array_val.length; i++)
        {
            if (blobit_value_copy(val->array_val.items[i], &new_val->array_val.items[i]) != BLOBIT_SUCCESS)
            {
                for (size_t j = 0; j < i; j++)
                    blobit_value_destroy(new_val->array_val.items[j]);
                free(new_val->array_val.items);
                free(new_val);
                return BLOBIT_ERROR_ALLOCATION_FAILED;
            }
        }
        break;
    case BLOB_TYPE_MAP:
        new_val->map_val.length = val->map_val.length;
        new_val->map_val.keys = calloc(val->map_val.length, sizeof(blobit_value_t*));
        new_val->map_val.values = calloc(val->map_val.length, sizeof(blobit_value_t*));
        if (!new_val->map_val.keys || !new_val->map_val.values)
        {
            free(new_val->map_val.keys);
            free(new_val->map_val.values);
            free(new_val);
            return BLOBIT_ERROR_ALLOCATION_FAILED;
        }
        for (size_t i = 0; i < val->map_val.length; i++)
        {
            if (blobit_value_copy(val->map_val.keys[i], &new_val->map_val.keys[i]) != BLOBIT_SUCCESS ||
                blobit_value_copy(val->map_val.values[i], &new_val->map_val.values[i]) != BLOBIT_SUCCESS)
            {
                for (size_t j = 0; j < i; j++)
                {
                    blobit_value_destroy(new_val->map_val.keys[j]);
                    blobit_value_destroy(new_val->map_val.values[j]);
                }
                free(new_val->map_val.keys);
                free(new_val->map_val.values);
                free(new_val);
                return BLOBIT_ERROR_ALLOCATION_FAILED;
            }
        }
        break;
    default:
        break;
    }

    *out_val = new_val;
    return BLOBIT_SUCCESS;
}

const char* blobit_error_message(int code)
{
    switch (code)
    {
    case BLOBIT_SUCCESS: return "success";
    case BLOBIT_FAILURE: return "general failure";
    case BLOBIT_ERROR_INVALID_TYPE: return "invalid type for operation";
    case BLOBIT_ERROR_MAP_KEY_NOT_FOUND: return "map key not found";
    case BLOBIT_ERROR_ARRAY_INDEX_OUT_OF_BOUNDS: return "array index out of bounds";
    case BLOBIT_ERROR_NULL_POINTER: return "null pointer argument";
    case BLOBIT_ERROR_ALLOCATION_FAILED: return "memory allocation failed";
    default: return "unknown error";
    }
}

int blobit_serialize_with(const blobit_value_t* val, const blobit_serializer_t* serializer, uint8_t** out_buf,
                          size_t* out_size)
{
    if (!val || !out_buf || !out_size)
        return BLOBIT_ERROR_NULL_POINTER;
    if (serializer && serializer->serialize)
        return serializer->serialize(val, serializer->user_data, out_buf, out_size);
    return blobit_default_serialize(val, NULL, out_buf, out_size);
}

int blobit_deserialize_with(const uint8_t* buf, size_t size, const blobit_serializer_t* serializer,
                            blobit_value_t** out_val)
{
    if (!buf || !out_val)
        return BLOBIT_ERROR_NULL_POINTER;
    if (serializer && serializer->deserialize)
        return serializer->deserialize(buf, size, serializer->user_data, out_val);
    return blobit_default_deserialize(buf, size, NULL, out_val);
}

int blobit_serialize(const blobit_value_t* val, uint8_t** out_buf, size_t* out_size)
{
    return blobit_serialize_with(val, NULL, out_buf, out_size);
}

int blobit_deserialize(const uint8_t* buf, size_t size, blobit_value_t** out_val)
{
    return blobit_deserialize_with(buf, size, NULL, out_val);
}

int blobit_serialize_to_file_with(const blobit_value_t* val, const char* filename, const blobit_serializer_t* serializer)
{
    if (!val || !filename)
        return BLOBIT_ERROR_NULL_POINTER;

    uint8_t* buf = NULL;
    size_t buf_size = 0;
    int res = blobit_serialize_with(val, serializer, &buf, &buf_size);
    if (res != BLOBIT_SUCCESS)
        return res;

    FILE* file = fopen(filename, "wb");
    if (!file)
    {
        free(buf);
        return BLOBIT_FAILURE;
    }

    size_t written = fwrite(buf, 1, buf_size, file);
    fclose(file);
    free(buf);

    if (written != buf_size)
        return BLOBIT_FAILURE;

    return BLOBIT_SUCCESS;
}

int blobit_deserialize_from_file_with(const char* filename, blobit_value_t** out_val, const blobit_serializer_t* serializer)
{
    if (!filename || !out_val)
        return BLOBIT_ERROR_NULL_POINTER;

    FILE* file = fopen(filename, "rb");
    if (!file)
        return BLOBIT_FAILURE;

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size < 0)
    {
        fclose(file);
        return BLOBIT_FAILURE;
    }

    uint8_t* buf = malloc(file_size);
    if (!buf)
    {
        fclose(file);
        return BLOBIT_ERROR_ALLOCATION_FAILED;
    }

    size_t read = fread(buf, 1, file_size, file);
    fclose(file);
    if (read != (size_t)file_size)
    {
        free(buf);
        return BLOBIT_FAILURE;
    }

    int res = blobit_deserialize_with(buf, read, serializer, out_val);
    free(buf);
    return res;
}

int blobit_serialize_to_file(const blobit_value_t* val, const char* filename)
{
    return blobit_serialize_to_file_with(val, filename, NULL);
}

int blobit_deserialize_from_file(const char* filename, blobit_value_t** out_val)
{
    return blobit_deserialize_from_file_with(filename, out_val, NULL);
}
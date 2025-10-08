//
// Created by Will Ballantine on 10/8/25.
//

#include "blobit/__private/default_serializer.h"
#include <string.h>
#include <stdlib.h>

#define BLOBIT_SIGNATURE 0x626C6F62u
#define BLOBIT_VERSION 0x01
#define BLOBIT_HEADER_SIZE 8

static void write_u32_le(uint8_t* buf, uint32_t v)
{
    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)((v >> 8) & 0xFF);
    buf[2] = (uint8_t)((v >> 16) & 0xFF);
    buf[3] = (uint8_t)((v >> 24) & 0xFF);
}

static uint32_t read_u32_le(const uint8_t* buf)
{
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

static void write_u64_le(uint8_t* buf, uint64_t v)
{
    for (int i = 0; i < 8; ++i) buf[i] = (uint8_t)((v >> (8 * i)) & 0xFF);
}

static uint64_t read_u64_le(const uint8_t* buf)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= ((uint64_t)buf[i]) << (8 * i);
    return v;
}

static int ensure_capacity(uint8_t** buf, size_t* cap, size_t need)
{
    if (*cap >= need) return 0;
    size_t new_cap = *cap ? *cap * 2 : 256;
    while (new_cap < need) new_cap *= 2;
    uint8_t* new_buf = realloc(*buf, new_cap);
    if (!new_buf) return -1;
    *buf = new_buf;
    *cap = new_cap;
    return 0;
}

static int serialize_value(const blobit_value_t* val, uint8_t** buf, size_t* cap, size_t* off)
{
    if (!val) return BLOBIT_ERROR_NULL_POINTER;
    if (ensure_capacity(buf, cap, *off + 1)) return BLOBIT_ERROR_ALLOCATION_FAILED;
    (*buf)[(*off)++] = (uint8_t)val->type;
    switch (val->type)
    {
    case BLOB_TYPE_INT:
        {
            if (ensure_capacity(buf, cap, *off + 8)) return BLOBIT_ERROR_ALLOCATION_FAILED;
            write_u64_le(*buf + *off, (uint64_t)val->int_val);
            *off += 8;
            break;
        }
    case BLOB_TYPE_FLOAT:
        {
            if (ensure_capacity(buf, cap, *off + 8)) return BLOBIT_ERROR_ALLOCATION_FAILED;
            union
            {
                double d;
                uint64_t u;
            } u;
            u.d = val->float_val;
            u.u = 0;
            write_u64_le(*buf + *off, u.u);
            *off += 8;
            break;
        }
    case BLOB_TYPE_BOOL:
        {
            if (ensure_capacity(buf, cap, *off + 1)) return BLOBIT_ERROR_ALLOCATION_FAILED;
            (*buf)[(*off)++] = val->bool_val ? 0x01 : 0x00;
            break;
        }
    case BLOB_TYPE_STRING:
        {
            size_t len = val->string_val ? strlen(val->string_val) : 0;
            if (ensure_capacity(buf, cap, *off + 4 + len)) return BLOBIT_ERROR_ALLOCATION_FAILED;
            write_u32_le(*buf + *off, (uint32_t)len);
            *off += 4;
            if (len)
                memcpy(*buf + *off, val->string_val, len);
            *off += len;
            break;
        }
    case BLOB_TYPE_ARRAY:
        {
            if (ensure_capacity(buf, cap, *off + 4)) return BLOBIT_ERROR_ALLOCATION_FAILED;
            write_u32_le(*buf + *off, (uint32_t)val->array_val.length);
            *off += 4;
            for (size_t i = 0; i < val->array_val.length; ++i)
            {
                int r = serialize_value(val->array_val.items[i], buf, cap, off);
                if (r != BLOBIT_SUCCESS) return r;
            }
            break;
        }
    case BLOB_TYPE_MAP:
        {
            if (ensure_capacity(buf, cap, *off + 4)) return BLOBIT_ERROR_ALLOCATION_FAILED;
            write_u32_le(*buf + *off, (uint32_t)val->map_val.length);
            *off += 4;
            for (size_t i = 0; i < val->map_val.length; ++i)
            {
                int r = serialize_value(val->map_val.keys[i], buf, cap, off);
                if (r != BLOBIT_SUCCESS) return r;
                r = serialize_value(val->map_val.values[i], buf, cap, off);
                if (r != BLOBIT_SUCCESS) return r;
            }
            break;
        }
    case BLOB_TYPE_NIL:
    default:
        break;
    }
    return BLOBIT_SUCCESS;
}

int blobit_default_serialize(const blobit_value_t* val, void* user_data, uint8_t** out_buf, size_t* out_size)
{
    (void)user_data;
    if (!val || !out_buf || !out_size) return BLOBIT_ERROR_NULL_POINTER;
    size_t cap = 256, off = 0;
    uint8_t* buf = malloc(cap);
    if (!buf) return BLOBIT_ERROR_ALLOCATION_FAILED;
    write_u32_le(buf, BLOBIT_SIGNATURE);
    buf[4] = BLOBIT_VERSION;
    buf[5] = buf[6] = buf[7] = 0;
    off = BLOBIT_HEADER_SIZE;
    int r = serialize_value(val, &buf, &cap, &off);
    if (r != BLOBIT_SUCCESS)
    {
        free(buf);
        return r;
    }
    *out_buf = buf;
    *out_size = off;
    return BLOBIT_SUCCESS;
}

static int deserialize_value(const uint8_t* buf, size_t size, size_t* off, blobit_value_t** out_val)
{
    if (*off >= size) return BLOBIT_FAILURE;
    uint8_t type = buf[(*off)++];
    blobit_value_t* val = NULL;
    int r = blobit_value_create(&val, (blobit_value_type_t)type);
    if (r != BLOBIT_SUCCESS) return r;
    switch (type)
    {
    case BLOB_TYPE_INT:
        {
            if (*off + 8 > size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            int64_t v = (int64_t)read_u64_le(buf + *off);
            val->int_val = v;
            *off += 8;
            break;
        }
    case BLOB_TYPE_FLOAT:
        {
            if (*off + 8 > size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            union
            {
                double d;
                uint64_t u;
            } u;
            u.d = 0;

            u.u = read_u64_le(buf + *off);
            val->float_val = u.d;
            *off += 8;
            break;
        }
    case BLOB_TYPE_BOOL:
        {
            if (*off >= size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            val->bool_val = buf[(*off)++] ? true : false;
            break;
        }
    case BLOB_TYPE_STRING:
        {
            if (*off + 4 > size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            uint32_t len = read_u32_le(buf + *off);
            *off += 4;
            if (*off + len > size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            val->string_val = malloc(len + 1);
            if (!val->string_val)
            {
                blobit_value_destroy(val);
                return BLOBIT_ERROR_ALLOCATION_FAILED;
            }
            memcpy(val->string_val, buf + *off, len);
            val->string_val[len] = '\0';
            *off += len;
            break;
        }
    case BLOB_TYPE_ARRAY:
        {
            if (*off + 4 > size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            uint32_t arrlen = read_u32_le(buf + *off);
            *off += 4;
            val->array_val.length = arrlen;
            val->array_val.capacity = arrlen;
            val->array_val.items = calloc(arrlen, sizeof(blobit_value_t*));
            if (arrlen && !val->array_val.items)
            {
                blobit_value_destroy(val);
                return BLOBIT_ERROR_ALLOCATION_FAILED;
            }
            for (uint32_t i = 0; i < arrlen; ++i)
            {
                int rr = deserialize_value(buf, size, off, &val->array_val.items[i]);
                if (rr != BLOBIT_SUCCESS)
                {
                    blobit_value_destroy(val);
                    return rr;
                }
            }
            break;
        }
    case BLOB_TYPE_MAP:
        {
            if (*off + 4 > size)
            {
                blobit_value_destroy(val);
                return BLOBIT_FAILURE;
            }
            uint32_t maplen = read_u32_le(buf + *off);
            *off += 4;
            val->map_val.length = maplen;
            val->map_val.capacity = maplen;
            val->map_val.keys = calloc(maplen, sizeof(blobit_value_t*));
            val->map_val.values = calloc(maplen, sizeof(blobit_value_t*));
            if ((maplen && (!val->map_val.keys || !val->map_val.values)))
            {
                blobit_value_destroy(val);
                return BLOBIT_ERROR_ALLOCATION_FAILED;
            }
            for (uint32_t i = 0; i < maplen; ++i)
            {
                int rr = deserialize_value(buf, size, off, &val->map_val.keys[i]);
                if (rr != BLOBIT_SUCCESS)
                {
                    blobit_value_destroy(val);
                    return rr;
                }
                rr = deserialize_value(buf, size, off, &val->map_val.values[i]);
                if (rr != BLOBIT_SUCCESS)
                {
                    blobit_value_destroy(val);
                    return rr;
                }
            }
            break;
        }
    case BLOB_TYPE_NIL:
    default:
        break;
    }
    *out_val = val;
    return BLOBIT_SUCCESS;
}

int blobit_default_deserialize(const uint8_t* buf, size_t size, void* user_data, blobit_value_t** out_val)
{
    (void)user_data;
    if (!buf || !out_val || size < BLOBIT_HEADER_SIZE) return BLOBIT_ERROR_NULL_POINTER;
    if (read_u32_le(buf) != BLOBIT_SIGNATURE || buf[4] != BLOBIT_VERSION) return BLOBIT_FAILURE;
    size_t off = BLOBIT_HEADER_SIZE;
    return deserialize_value(buf, size, &off, out_val);
}

#include <blobit/blobit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 10000
#define ARR_SIZE 100
#define MAP_SIZE 100

static void fill_array(blobit_value_t *arr, int n) {
    for (int i = 0; i < n; ++i) {
        blobit_value_t *v;
        blobit_value_create(&v, BLOB_TYPE_INT);
        blobit_value_assign_int(v, i);
        blobit_array_append(arr, v);
    }
}

static void fill_map(blobit_value_t *map, int n) {
    for (int i = 0; i < n; ++i) {
        blobit_value_t *k, *v;
        blobit_value_create(&k, BLOB_TYPE_STRING);
        char buf[32];
        snprintf(buf, sizeof(buf), "key%d", i);
        blobit_value_assign_string(k, buf);
        blobit_value_create(&v, BLOB_TYPE_FLOAT);
        blobit_value_assign_float(v, (double)i * 1.5);
        blobit_map_insert(map, k, v);
    }
}

static double elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1e6;
}

int main(void) {
    struct timespec t0, t1;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    blobit_value_t *root;
    blobit_value_create(&root, BLOB_TYPE_ARRAY);
    for (int i = 0; i < N; ++i) {
        blobit_value_t *arr, *map;
        blobit_value_create(&arr, BLOB_TYPE_ARRAY);
        fill_array(arr, ARR_SIZE);
        blobit_value_create(&map, BLOB_TYPE_MAP);
        fill_map(map, MAP_SIZE);
        blobit_array_append(root, arr);
        blobit_array_append(root, map);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Created %d arrays/maps in %.2f ms\n", N, elapsed_ms(t0, t1));

    uint8_t *buf = NULL;
    size_t buf_size = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int res = blobit_serialize(root, &buf, &buf_size);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (res != BLOBIT_SUCCESS) {
        fprintf(stderr, "Serialize failed: %s\n", blobit_error_message(res));
        blobit_value_destroy(root);
        return 1;
    }
    printf("Serialized size: %zu bytes in %.2f ms\n", buf_size, elapsed_ms(t0, t1));

    blobit_value_t *copy = NULL;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    res = blobit_deserialize(buf, buf_size, &copy);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    free(buf);
    if (res != BLOBIT_SUCCESS) {
        fprintf(stderr, "Deserialize failed: %s\n", blobit_error_message(res));
        blobit_value_destroy(root);
        return 1;
    }
    printf("Deserialized in %.2f ms\n", elapsed_ms(t0, t1));

    clock_gettime(CLOCK_MONOTONIC, &t0);
    blobit_value_destroy(root);
    blobit_value_destroy(copy);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Destroyed in %.2f ms\n", elapsed_ms(t0, t1));

    return 0;
}

# blobit—Fast extendable serialization for C

`blobit` is a lightweight serialization library for C.
Its main goal is to be easy to use and easy to extend.

With blobit you can create your own serialization formats, and use them in your projects.

blobit is under 1.5k lines of code, and the API is simple.

## Example

```c++
#include <blobit/blobit.h>
#include <stdio.h>

int main() {
    blobit_value_t *root;
    blobit_value_create(&root, BLOB_TYPE_MAP);

    blobit_value_t *key, *val;
    blobit_value_create(&key, BLOB_TYPE_STRING);
    blobit_value_assign_string(key, "hello");
    blobit_value_create(&val, BLOB_TYPE_STRING);
    blobit_value_assign_string(val, "world");
    blobit_map_insert(root, key, val);

    printf("Original value:\n");
    blobit_value_print(root, stdout);
    printf("\n");

    uint8_t *buf = NULL;
    size_t buf_size = 0;
    blobit_serialize(root, &buf, &buf_size);

    blobit_value_t *copy = NULL;
    blobit_deserialize(buf, buf_size, &copy);
    printf("Deserialized value:\n");
    blobit_value_print(copy, stdout);
    printf("\n");

    blobit_value_destroy(root);
    blobit_value_destroy(copy);
    free(buf);
    return 0;
}
```

## Features
- Simple, C99 API
- Fast binary serialization
- Extensible: plug in your own formats
- Supports int, float, bool, string, array, map
- Zero dependencies

## CLI

`blobit-cli` can pretty-print, check, and convert blobit files:

```
blobit print <file>           # Pretty-print a blobit file
blobit dump <file>            # Dump raw bytes in hex
blobit check <file>           # Validate a blobit file
blobit deserialize <in> <out> # Convert to text
```

## Serialization format

See [notes/serialization.md](notes/serialization.md) for the full default binary format.

## License
MIT

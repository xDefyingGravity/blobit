Blobit serialization

`blobit-cli`

## Blobit Default Serializer Format

```xml
<header>
    <signature>0x626C6F62</signature>
    <version>1 byte (0x01)</version>
    <zero>3 bytes of 0x00</zero>
</header>
<type> 1 byte

<payload> variable </payload>
```
Type-specific Payloads
	•	BLOB_TYPE_INT

8 bytes: int64_t (little-endian)

	•	BLOB_TYPE_FLOAT

8 bytes: double (IEEE 754, little-endian)


	•	BLOB_TYPE_BOOL

1 byte: 0x00 = false, 0x01 = true


	•	BLOB_TYPE_STRING

4 bytes: uint32_t length N (little-endian)
N bytes: UTF-8 string data (no null terminator)


	•	BLOB_TYPE_ARRAY

4 bytes: uint32_t length L (number of items)
Repeat L times: serialized items (each item is a full blobit value)


	•	BLOB_TYPE_MAP

4 bytes: uint32_t length M (number of key-value pairs)
Repeat M times:
  serialized key (blobit value)
  serialized value (blobit value)


Notes
	•	Multi-byte integers and floats are little-endian.
	•	Arrays and maps are recursively serialized using the same format.
	•	This format is compact and fully self-contained: each value knows its type and length.
	•	Signature at the start allows detection of Blobit data streams.
## k_json

`k_json` — small, in-repo JSON value representation and helpers (work in progress).

### Overview

The `k_json` module provides a compact value type (`K_jsonValue`) and a small set of helpers to parse, inspect and free JSON values used by the server. The module is intentionally minimal: it models JSON as a typed value with convenient accessors for boolean, integer, float and string values, plus array/object containers.

Important: the JSON parser and some convenience helpers are currently a work-in-progress in `core/k_json.c`. The API documented below reflects the header-level contract (`core/k_json.h`) — callers should treat parsing functions as not-yet-implemented until the C sources are completed.

### Features

- Typed JSON value representation (`K_jsonValue`) with enum discriminant `K_jsonType`.
- Accessor helpers for primitive values: `k_json_bool`, `k_json_int`, `k_json_float`, `k_json_string`.
- Object/array containers (arrays of `K_jsonValue *` and key/value arrays for objects).
- Memory management helper: `k_json_free` to recursively free a value.
- Error identifiers declared for consumers: `JSON_PARSE_ERROR`, `JSON_INVALID_TYPE` and pointer-sentinels via `K_DEFINE_ERR` (see `core/k_json.h`).

### Usage (high-level)

1. Include the header:

```c
#include "k_json.h"
```

2. Parse a JSON buffer (WIP — parser not implemented yet):

```c
K_jsonValue *val = k_json_parse(json_buf, json_len);
if (!val) {
    // parse failed or parser not implemented yet
}
// use accessors, then free
k_json_free(val);
```

3. Access primitive values using helpers:

```c
int b = k_json_bool(value);      // 1/0 or -1 on invalid
int i = k_json_int(value);       // integer value or (per header) NAN/invalid sentinel
double f = k_json_float(value);  // float or NAN on invalid
const char *s = k_json_string(value); // string pointer or NULL on invalid
```

### API Reference

#### Types

- `K_jsonType` (enum)
  - `K_JSON_NULL`
  - `K_JSON_BOOL`
  - `K_JSON_INT`
  - `K_JSON_FLOAT`
  - `K_JSON_STRING`
  - `K_JSON_ARRAY`
  - `K_JSON_OBJECT`

- `K_jsonValue` (struct)
  - `K_jsonType type` — discriminant.
  - Union members depending on type:
    - `int bool_value`
    - `int int_value`
    - `float float_value`
    - `char *string_value` (heap owned, freed by `k_json_free`)
    - `array` struct: `K_jsonValue **items; size_t count;`
    - `object` struct: `char **keys; K_jsonValue **values; size_t count;`

#### Macros / Errors

- `JSON_PARSE_ERROR` — numeric identifier; `K_DEFINE_ERR(JSON_PARSE_ERROR)` is used in the header to provide a pointer-style sentinel.
- `JSON_INVALID_TYPE` — numeric identifier for invalid-type errors; also wrapped by `K_DEFINE_ERR` in the header.

Note: The header defines these macros and then invokes `K_DEFINE_ERR(...)`. The header also undefines the numeric macros at the end to avoid leaking them as preprocessor symbols.

#### Functions

- `K_jsonValue* k_json_parse(const char *json_str, size_t json_str_len);`
  - Parse the input JSON buffer and return a newly-allocated `K_jsonValue*` on success. The header documents that this returns `NULL` on failure. Implementation: currently a stub in `core/k_json.c` (WIP).

- `char* k_json_serialize(K_jsonValue* value);`
  - Serialize `value` to a newly-allocated string (caller frees). Declaration present in header; implementation may be added later.

- `K_jsonValue* k_json_get_object(K_jsonValue* obj, const char* key);`
  - Lookup a key in an object-valued `K_jsonValue`. Declared in the header; implementation currently a stub in `core/k_json.c` (WIP).

- `void k_json_free(K_jsonValue* value);`
  - Recursively free a JSON value and its contents. Safe to call with `NULL`.

- `int k_json_bool(K_jsonValue* value);`
  - Return boolean value (1 for true, 0 for false) or -1 for invalid input (per header documentation).

- `double k_json_float(K_jsonValue* value);`
  - Return float value, or `NAN` for invalid input.

- `int k_json_int(K_jsonValue* value);`
  - Return integer value; header documentation notes `NAN` for invalid, but the function returns an `int` and should be treated as returning an implementation-defined sentinel on invalid input — check the header/source for exact sentinel semantics.

- `const char* k_json_string(K_jsonValue* value);`
  - Return pointer to the string value or `NULL` for invalid input.

### Ownership and error semantics

- Functions that return allocated objects (`k_json_parse`) transfer ownership to the caller; callers must free with `k_json_free`.
- `k_json_free` recursively frees object and array contents and their string/keys; it is safe to call with `NULL`.
- Accessor functions return simple values or sentinels on invalid input; callers should check return values per the header's documented sentinels.

### Status / Notes

- The JSON module is explicitly marked WIP in the sources — `k_json_parse` and `k_json_get_object` are currently unimplemented stubs in `core/k_json.c`. This documentation intentionally sticks to the header's declarations and does not guess parser behavior beyond what the header states.
- When the parser and serializer are implemented, consider adding small unit tests exercising parsing, serialization round-trips, and object lookups.

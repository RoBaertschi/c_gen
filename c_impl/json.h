#ifndef JSON_H_INC
#define JSON_H_INC

// This is json.h by RoBaertschi, a simple, single header json parser. Written in C99.
// Define JSON_IMPLEMENTATION to enable the implementation
//
// Defines:
//  JSON_ASSERT(condition, message)
//  Default: assert(condition && message)
//  Includes by default: assert.h
//  Assert function, should check if condition is true. It should abort in debug mode
//
//  JSON_STATIC_ASSERT
//  Default: static_assert
//  Includes by default: assert.h
//  Do some static assert
//
//  JSON_GROWTH_FACTOR
//  Default: 2
//  The multiplier by which the internal hash map capacity increases, when it runs out of capacity. A good value is 1.5 - 2.
//
//  JSON_MAX_LOAD_FACTOR
//  Default: 0.7
//  The precentage at which the internal hash map increases the bucket capacity
//
//  JSON_INITIAL_BUCKET_SIZE
//  Default: 16
//  This is the Default size of a Bucket for a json_object. The larger the Bucket Size, the faster the access but higher Memory Use.
//
//  JSON_{MALLOC,CALLOC,REALLOC,FREE}
//  Defaults: stdlib.h
//  These are functions used to allocate memory. If you want to use some sort of custom allocator, this is the place to add it.
//  NOTE: There might be a point where we add full custom allocator support with user data support.
//

#include <string.h>
#ifndef JSON_STATIC_ASSERT
#include <assert.h>
#define JSON_STATIC_ASSERT static_assert
#endif // JSON_STATIC_ASSERT

#ifndef JSON_INITIAL_BUCKET_SIZE
#define JSON_INITIAL_BUCKET_SIZE 16
#endif // JSON_INITIAL_BUCKET_SIZE

// C11
// JSON_STATIC_ASSERT(JSON_INITIAL_BUCKET_SIZE > 0, "JSON_INITIAL_BUCKET_SIZE has to be larger than 0");

#ifndef JSON_GROWTH_FACTOR
#define JSON_GROWTH_FACTOR 2
#endif // JSON_GROWTH_FACTOR

// C11
// JSON_STATIC_ASSERT(JSON_GROWTH_FACTOR > 0, "JSON_GROWTH_FACTOR has to be larger than 0");

#ifndef JSON_MAX_LOAD_FACTOR
#define JSON_MAX_LOAD_FACTOR 0.7
#endif // JSON_MAX_LOAD_FACTOR


#ifndef JSON_MALLOC
#include <stdlib.h>
#define JSON_MALLOC(size) malloc(size)
#endif // JSON_MALLOC

#ifndef JSON_CALLOC
#include <stdlib.h>
#define JSON_CALLOC(count,size) calloc(count, size)
#endif // JSON_CALLOC

#ifndef JSON_REALLOC
#include <stdlib.h>
#define JSON_REALLOC(ptr, size) realloc(ptr, size)
#endif // JSON_REALLOC

#ifndef JSON_FREE
#include <stdlib.h>
#define JSON_FREE(ptr) free(ptr)
#endif // JSON_FREE

#ifndef JSON_ASSERT
#include <assert.h>
#define JSON_ASSERT(condition, message) assert(condition && message)
#endif // JSON_ASSERT 

#include <stddef.h>
#include <stdbool.h>

enum json_type {
    // Invalid json value, a zero constructed value is invalid
    JSON_INVALID,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_STRING,
    JSON_NULL,
};


// array with len 0 do not have any heap memory, items will be set to NULL
struct json_array {
    size_t len;
    struct json_value *items;
};

struct json_string {
    size_t len;
    unsigned char *data;
};

// NOTE: The content of json_object is internal, you should not use it.
struct json_object {
    // NOTE: Internal API, to access please use the public api functions
    struct json__hash_map* _hm;
};

// NOTE: The content of json_object_iterator is internal, you should not use it.
// WARN: Do not modify the object while iterating, some weird stuff can happen if you do.
struct json_object_iterator {
    size_t _bucket_index;
    struct json__hash_map_entry *_current_entry;
    struct json__hash_map *_hm;
};

// WARN: Do not modify the object while iterating, some weird stuff can happen if you do.
struct json_object_entry {
    // The key is not a pointer, because it already is just a pointer with a length
    struct json_string key;
    struct json_value* value;
    bool found;
};

struct json_value {
    enum json_type type;
    // NOTE: JSON_NULL does not have any data.
    union json_data {
        bool boolean;
        double number;
        struct json_object object;
        struct json_array array;
        struct json_string string;
    } data ;
};


//------------------------------
// OBJECT PUBLIC API
//------------------------------

// Allocates a new object, use the other json_object_* functions to add and remove attributes.
// If any of the allocations fails, ok will be set to false. On success, ok will be set to true.
// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_object json_object_create(bool *ok);
struct json_value json_object_to_value(struct json_object obj);

// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_object json_object_copy(struct json_object obj, bool *ok);
// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_value json_object_copyv(struct json_object obj, bool *ok);

// Modifying the object

// Sets or inserts on demand, a value for a key
// The key will be copied, the value will be taken as is, the value will be owned by the object afterwards.
// Returns false if there was an allocation error.
bool json_object_set(struct json_object *obj, struct json_string key, struct json_value value);

// Gets the value at the key.
struct json_value json_object_get(struct json_object *obj, struct json_string key, bool *found);

// Deletes key from the object
// Returns true if the key was found, false if not
bool json_object_del(struct json_object *obj, struct json_string key);

// WARN: Do not modify the object while iterating, some weird stuff can happen if you do.
struct json_object_iterator json_object_iterator_create(struct json_object *obj);
// WARN: Do not modify the object while iterating, some weird stuff can happen if you do.
struct json_object_entry json_object_iterator_next(struct json_object_iterator* iterator);

//------------------------------
// STRING PUBLIC API
//------------------------------

// Makes a copy of the str
// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_string json_string_create(unsigned char *str, size_t len, bool *ok);
// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_value json_string_createv(unsigned char *str, size_t len, bool *ok);

struct json_string json_string_create_empty(void);
struct json_value json_string_create_emptyv(void);

// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_string json_string_create_cstr(unsigned char *str, bool *ok);
// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_value json_string_create_cstrv(char *str, bool *ok);

// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_string json_string_copy(struct json_string str, bool *ok);
// NOTE: ok has to be provided, if it null, an assertion will fail.
struct json_value json_string_copyv(struct json_string str, bool *ok);

// Do not use the str passed into this function, the returnec value has taken ownership of str
struct json_value json_string_to_value(struct json_string str);

#define JSON_STR(cstr) (struct json_string) { .data = (unsigned char *) cstr, .len = sizeof(cstr) / sizeof(cstr[0]) }

//------------------------------
// ARRAY PUBLIC API
//------------------------------

// Create a new, empty array. This is not allocated.
struct json_array json_array_create(void);

// Allocates a new array and copies the elements over, mostly for a utility macro
// If the arr.len > 0 and the return.items == NULL. Then the allocation failed. When a allocation fails, the returned array will have a length of 0.
struct json_array json_array_copy(struct json_array arr, bool *ok);
struct json_value json_array_copyv(struct json_array arr, bool *ok);

// Allocates a new array and copies both left and right into it
// If the left.len + right.len > 0 and the return.items == NULL. Then the allocation failed. When a allocation fails, the returned array will have a length of 0.
struct json_array json_array_concat(struct json_array left, struct json_array right, bool *ok);
struct json_value json_array_concatv(struct json_array left, struct json_array right, bool *ok);

// This macro allows for easy creation of a array's with values, because currently json_array's are immutable (100% not because I am lazy to implement the proper functions or something /s).
#define JSON_CREATE_ARRAY(ok, ...) json_array_copy((struct json_array) { .items = (struct json_value[]){ __VA_ARGS__ }, .len = sizeof((struct json_value[]){ __VA_ARGS__ }) / sizeof(struct json_value) }, (ok))

struct json_value json_null(void);

struct json_value json_array_to_value(struct json_array arr);

//------------------------------
// OTHER CREATE PUBLIC API
//------------------------------

// This function cannot fail
struct json_value json_boolean(bool value);
// This function cannot fail
struct json_value json_number(double value);
// This function cannot fail
struct json_value json_null(void);

struct json_value json_value_copy(struct json_value value, bool *ok);

// Free
void json_value_delete(struct json_value value);
void json_object_delete(struct json_object object);
void json_array_delete(struct json_array array);
void json_string_delete(struct json_string string);

#ifdef JSON_IMPLEMENTATION


//------------------------------
// INTERNAL API
//------------------------------

// hash map
static struct json__hash_map* json__hm_create(void);
static void json__hm_delete(struct json__hash_map* map);
static bool json__hash_map_insert(struct json__hash_map *hm, struct json_string key, struct json_value value);
static struct json__hash_map_entry* json__hash_map_get(struct json__hash_map *hm, struct json_string key);
static bool json__hash_map_grow(struct json__hash_map *hm);
static double json__hash_map_load_factor(struct json__hash_map *hm);
static void json__hash_map_entry_delete(struct json__hash_map_entry* entry);
static bool json__hash_map_set(struct json__hash_map *hm, struct json_string key, struct json_value value);
static bool json__hash_map_delete(struct json__hash_map *hm, struct json_string key);
static struct json__hash_map *json__hash_map_copy(struct json__hash_map *hm);

// hash map entry
static struct json__hash_map_entry json__hash_map_entry_copy(struct json__hash_map_entry entry, struct json__hash_map_entry *new_collisions, struct json__hash_map_entry *old_collisions, bool *ok);
static bool json__hash_map_entry_valid(struct json__hash_map_entry *entry);

// string
static bool json__string_eq(struct json_string first, struct json_string second);

// Internal Hash Map for strings to json_value
struct json__hash_map {
    struct json__hash_map_entry *bucket;
    size_t bucket_size;
    size_t bucket_cap;

    // This is a linked list
    struct json__hash_map_entry *collisions;
    size_t collisions_size;
    size_t collisions_cap;
};

struct json__hash_map_entry {
    struct json_string key;
    struct json_value value;
    struct json__hash_map_entry* next;
};



//------------------------------
// PUBLIC API IMPL
//------------------------------

struct json_object_iterator json_object_iterator_create(struct json_object *obj) {
    return (struct json_object_iterator) {
        ._bucket_index = 0,
        ._current_entry = NULL,
        ._hm = obj->_hm,
    };
}

struct json_object_entry json_object_iterator_next(struct json_object_iterator* iterator) {
    if (iterator->_current_entry && iterator->_current_entry->next) {
        iterator->_current_entry = iterator->_current_entry->next;
        return (struct json_object_entry) {
            .value = &iterator->_current_entry->value,
            .key = iterator->_current_entry->key,
            .found = true,
        };
    }

    for (size_t i = iterator->_bucket_index; i < iterator->_hm->bucket_cap; i++) {
        iterator->_current_entry = &iterator->_hm->bucket[iterator->_bucket_index];
        if (json__hash_map_entry_valid(iterator->_current_entry)) {
            return (struct json_object_entry) {
                .value = &iterator->_current_entry->value,
                .key = iterator->_current_entry->key,
                .found = true,
            };
        }
    }
    return (struct json_object_entry) {
        .found = false,
    };
}

struct json_string json_string_create(unsigned char *str, size_t len, bool *ok) {
    assert(ok != NULL);

    if (len <= 0) {
        *ok = true;
        return json_string_create_empty();
    }

    struct json_string new_str = {
        .data = malloc(len),
        .len = len,
    };

    if (new_str.data == NULL) {
        *ok = false;
        new_str.len = 0;
        return new_str;
    }
    *ok = true;

    for (size_t i = 0; i < len; i++) {
        new_str.data[i] = str[i];
    }

    return new_str;
}

// NOTE: Copies the string
// The string has to be longer than 0
// Returns an empty string with NULL as data when the allocation fails
struct json_value json_string_createv(unsigned char *str, size_t len, bool *ok) {
    return json_string_to_value(json_string_create(str, len, ok));
}

inline struct json_string json_string_create_empty(void) {
    return (struct json_string) {0};
}

inline struct json_value json_string_create_emptyv(void) {
    return json_string_to_value(json_string_create_empty());
}

// NOTE: Copies the string
// The string has to be longer than 0
// Returns an empty string with NULL as data when the allocation fails
struct json_string json_string_create_cstr(unsigned char *str, bool *ok) {
    return json_string_create(str, strlen((char*) str), ok);
}
// NOTE: Copies the string
// The string has to be longer than 0
// Returns an empty string with NULL as data when the allocation fails
struct json_value json_string_create_cstrv(char *str, bool *ok) {
    return json_string_createv((unsigned char*)str, strlen(str), ok);
}

// NOTE: Copies the string
// The string has to be longer than 0
// Returns an empty string with NULL as data when the allocation fails
struct json_string json_string_copy(struct json_string str, bool *ok) {
    return json_string_create(str.data, str.len, ok);
}

// NOTE: Copies the string
// The string has to be longer than 0
// Returns an empty string with NULL as data when the allocation fails
struct json_value json_string_copyv(struct json_string str, bool *ok) {
    return json_string_to_value(json_string_create(str.data, str.len, ok));
}

struct json_value json_string_to_value(struct json_string str) {
    return (struct json_value) {
        .type = JSON_STRING,
        .data.string = str,
    };
}

// Create object

// Allocates a new object, use the other json_object_* functions to add and remove attributes.
//
struct json_object json_object_create(bool *ok) {
    assert(ok != NULL);
    struct json__hash_map *hm = json__hm_create();
    if (hm == NULL) {
        *ok = false;
    } else {
        *ok = true;
    }
    return (struct json_object) {
        ._hm = hm,
    };
}

struct json_value json_object_to_value(struct json_object obj) {
    return (struct json_value) {
        .type = JSON_OBJECT,
        .data.object = obj,
    };
}

struct json_object json_object_copy(struct json_object obj, bool *ok) {
    assert(ok != NULL);
    struct json_object new_obj = {
        ._hm = json__hash_map_copy(obj._hm),
    };

    if (new_obj._hm == NULL) {
        *ok = false;
    } else {
        *ok = true;
    }

    return new_obj;
}

struct json_value json_object_copyv(struct json_object obj, bool *ok) {
    return json_object_to_value(json_object_copy(obj, ok));
}

bool json_object_set(struct json_object *obj, struct json_string key, struct json_value value) {
    if (json__hash_map_set(obj->_hm, key, value)) {
        return true;
    }

    // Insert if it does not exist.
    return json__hash_map_insert(obj->_hm, key, value);
}

// Gets the value at the key.
struct json_value json_object_get(struct json_object *obj, struct json_string key, bool *found) {
    struct json__hash_map_entry *entry = json__hash_map_get(obj->_hm, key);
    if (entry == NULL) {
        *found = false;
        return (struct json_value) {0};
    }
    *found = true;
    return entry->value;
}

bool json_object_del(struct json_object *obj, struct json_string key) {
    return json__hash_map_delete(obj->_hm, key);
}

// Create array

// Allocates a new array, use the other json_array_* functions to add and remove elements.
struct json_array json_array_create(void) {
    return (struct json_array) {
        .len = 0,
        .items = NULL,
    };
}

// Allocates a new array and copies both left and right into it
// If the left.len + right.len > 0 and the return.items == NULL. Then the allocation failed. When a allocation fails, the returned array will have a length of 0.
struct json_array json_array_concat(struct json_array left, struct json_array right, bool *ok) {
    struct json_value *new_items = JSON_MALLOC(sizeof(*new_items) * (left.len + right.len));
    if (new_items == NULL) {
        *ok = false;
        return json_array_create();
    }

    *ok = true;

    for (size_t i = 0; i < left.len; i++) {
        new_items[i] = left.items[i];
    }

    for (size_t i = 0; i < right.len; i++) {
        new_items[i + left.len] = right.items[i];
    }

    return (struct json_array) {
        .items = new_items,
        .len = left.len + right.len,
    };
}

struct json_value json_array_concatv(struct json_array left, struct json_array right, bool *ok) {
    return json_array_to_value(json_array_concat(left, right, ok));
}

// Allocates a new array and copies the elements over, mostly for a utility macro
// If the arr.len > 0 and the return.items == NULL. Then the allocation failed. When a allocation fails, the returned array will have a length of 0.
struct json_array json_array_copy(struct json_array arr, bool *ok) {
    struct json_value *items = JSON_MALLOC(arr.len * sizeof(*items));


    if (items == NULL) {
        *ok = false;
        // Return a empty array
        return json_array_create();
    }

    *ok = true;

    for (size_t i = 0; i < arr.len; i++) {
        items[i] = arr.items[i];
    }

    return (struct json_array) {
        .len = arr.len,
        .items = items,
    };
}

struct json_value json_array_copyv(struct json_array arr, bool *ok) {
    return json_array_to_value(json_array_copy(arr, ok));
}

struct json_value json_array_to_value(struct json_array arr) {
    return (struct json_value) {
        .type = JSON_ARRAY,
        .data.array = arr,
    };
}


struct json_value json_boolean(bool value) {
    return (struct json_value) {
        .type = JSON_BOOLEAN,
        .data.boolean = value,
    };
}

struct json_value json_number(double value) {
    return (struct json_value) {
        .type = JSON_NUMBER,
        .data.number = value,
    };
}

struct json_value json_null(void) {
    return (struct json_value) {
        .type = JSON_NULL,
    };
}

struct json_value json_value_copy(struct json_value value, bool *ok) {
    assert(ok != NULL);
    switch (value.type) {
        case JSON_OBJECT:
            return json_object_copyv(value.data.object, ok);
        case JSON_ARRAY:
            return json_array_copyv(value.data.array, ok);
        case JSON_STRING:
            return json_string_copyv(value.data.string, ok);
        case JSON_NUMBER:
        case JSON_BOOLEAN:
        case JSON_NULL:
        case JSON_INVALID:
            return value;
    }
}

void json_value_delete(struct json_value value) {
    switch (value.type) {
        case JSON_OBJECT:
            json_object_delete(value.data.object);
            break;
        case JSON_ARRAY:
            json_array_delete(value.data.array);
            break;
        case JSON_STRING:
            json_string_delete(value.data.string);
            break;
        case JSON_NUMBER:
        case JSON_BOOLEAN:
        case JSON_NULL:
        case JSON_INVALID:
            break;
    }
}

void json_object_delete(struct json_object object) {}

void json_array_delete(struct json_array array) {
    for (size_t i = 0; i < array.len; i++) {
        json_value_delete(array.items[i]);
    }
    JSON_FREE(array.items);
}

void json_string_delete(struct json_string string) {
    if (0 < string.len) {
        JSON_FREE(string.data);
    }
}

//------------------------------
// INTERNAL HASH MAP FACILITIES
//------------------------------

// Generate a hash value
static size_t json__hash(unsigned char const * str, size_t len) {
    size_t hash = 5381;

    for (size_t i = 0; i < len; i++) {
        hash = hash * 33 ^ str[i];
    }

    return hash;
}

// Returns NULL if an allocation failed
static struct json__hash_map* json__hm_create(void) {
    struct json__hash_map* ptr = JSON_MALLOC(sizeof(*ptr));
    if (ptr == NULL) {
        return NULL;
    }

    struct json__hash_map_entry *bucket = JSON_CALLOC(JSON_INITIAL_BUCKET_SIZE, sizeof(*bucket));
    if (bucket == NULL) {
        return NULL;
    }

    struct json__hash_map_entry *collisions = JSON_CALLOC(JSON_INITIAL_BUCKET_SIZE, sizeof(*collisions));

    *ptr = (struct json__hash_map) {
        .bucket_cap = JSON_INITIAL_BUCKET_SIZE,
        .bucket = bucket,

        .collisions = collisions,
        .collisions_cap = JSON_INITIAL_BUCKET_SIZE,
    };

    return ptr;
}

// Deletes all the values, keys and the internal memory of the hash map
static void json__hm_delete(struct json__hash_map* map) {
    for (size_t i = 0; i < map->collisions_size; i++) {
        if (json__hash_map_entry_valid(&map->collisions[i])) {
            json_string_delete(map->collisions[i].key);
            json_value_delete(map->collisions[i].value);
        }
    }

    for (size_t i = 0; i < map->bucket_cap; i++) {
        if (json__hash_map_entry_valid(&map->bucket[i])) {
            json_string_delete(map->bucket[i].key);
            json_value_delete(map->bucket[i].value);
        }
    }

    JSON_FREE(map->collisions);
    JSON_FREE(map->bucket);
    JSON_FREE(map);
}

static bool json__string_eq(struct json_string first, struct json_string second) {
    if (first.len != second.len) {
        return false;
    }

    for (size_t i = 0; i < first.len; i++) {
        if (first.data[i] != second.data[i]) {
            return false;
        }
    }

    return true;
}

// NOTE: Returns NULL if not found
static struct json__hash_map_entry* json__hash_map_get(struct json__hash_map *hm, struct json_string key) {
    size_t hash = json__hash(key.data, key.len);
    size_t index = hash % hm->bucket_cap;
    struct json__hash_map_entry *entry = &hm->bucket[index];

    if (!json__hash_map_entry_valid(entry)) {
        return NULL;
    }

    if (json__string_eq(entry->key, key)) {
        return entry;
    }

    while (entry->next != NULL) {
        entry = entry->next;
        if (json__string_eq(entry->key, key)) {
            return entry;
        }
    }

    return NULL;
}

// This function creates a new hash map, inserts all elements of the old one into it and then replaces the old one.
// It returns false on a allocation failiure
static bool json__hash_map_grow(struct json__hash_map *hm) {
    size_t new_cap = hm->bucket_cap * JSON_GROWTH_FACTOR;
    struct json__hash_map_entry *bucket = JSON_CALLOC(new_cap, sizeof(*bucket));
    struct json__hash_map_entry *collisions = JSON_CALLOC(new_cap, sizeof(*collisions));
    struct json__hash_map new_hm = {
        .bucket = bucket,
        .bucket_cap = new_cap,

        .collisions = collisions,
        .collisions_cap = new_cap,
    };

    for (size_t i = 0; i < hm->bucket_cap; i++) {
        struct json__hash_map_entry *entry = &hm->bucket[i];
        if (json__hash_map_entry_valid(entry)) {
            continue;
        }

        if(!json__hash_map_insert(&new_hm, entry->key, entry->value)) {
            JSON_FREE(bucket);
            JSON_FREE(collisions);
            return false;
        }
        while (entry->next != NULL) {
            entry = entry->next;
            if(!json__hash_map_insert(&new_hm, entry->key, entry->value)) {
                JSON_FREE(bucket);
                JSON_FREE(collisions);
                return false;
            }
        }
    }

    json__hm_delete(hm);
    *hm = new_hm;

    return true;
}

static double json__hash_map_load_factor(struct json__hash_map *hm) {
    return (double)hm->bucket_size / (double)hm->bucket_cap;
}

// NOTE: Returns false on allocation failiure
// The hash map stays valid even if the insert fails
// The key will be copied. You won't have to keep it alive.
static bool json__hash_map_insert(struct json__hash_map *hm, struct json_string key, struct json_value value) {
    bool ok = true;
    struct json__hash_map_entry new_entry = {
        .key = json_string_copy(key, &ok),
        .value = value,
    };

    if (!ok) {
        return false;
    }

    size_t hash = json__hash(new_entry.key.data, new_entry.key.len);
    size_t index = hash % hm->bucket_cap;
    struct json__hash_map_entry *entry = &hm->bucket[index];
    if (json__hash_map_entry_valid(entry)) {
        while (entry->next != NULL) {
            entry = entry->next;
        }

        if (hm->collisions_cap <= hm->collisions_size+1) {
            // NOTE: Why sizeof(*ptr)? this way, we can just change the type without having to change each sizeof
            size_t new_size = hm->collisions_cap * JSON_GROWTH_FACTOR * sizeof(*hm->collisions);
            struct json__hash_map_entry *new_collisions = JSON_REALLOC(hm->collisions, new_size);
            if (new_collisions == NULL) {
                return false;
            }
            hm->collisions = new_collisions;
            hm->collisions_cap = new_size;
        }

        hm->collisions[hm->collisions_size] = new_entry;
        entry->next = &hm->collisions[hm->collisions_size];
        hm->collisions_size += 1;
    } else {
        *entry = new_entry;
    }
    hm->bucket_size += 1;

    if (json__hash_map_load_factor(hm) > JSON_MAX_LOAD_FACTOR) {
        json__hash_map_grow(hm);
    }
    return true;
}

static void json__hash_map_entry_delete(struct json__hash_map_entry* entry) {
    json_string_delete(entry->key);
    json_value_delete(entry->value);
}

// Updates the value at the key. If the key does not exist, false is returned.
static bool json__hash_map_set(struct json__hash_map *hm, struct json_string key, struct json_value value) {
    struct json__hash_map_entry *entry = json__hash_map_get(hm, key);
    if (entry == false) {
        return false;
    }

    json_value_delete(entry->value);
    entry->value = value;

    return true;
}


// Returns false if we could not find the key
static bool json__hash_map_delete(struct json__hash_map *hm, struct json_string key) {
    size_t hash = json__hash(key.data, key.len);
    size_t index = hash % hm->bucket_cap;
    struct json__hash_map_entry *entry = &hm->bucket[index];

    if (json__string_eq(entry->key, key)) {
        json__hash_map_entry_delete(entry);
        if (entry->next) {
            *entry = *entry->next;
        } else {
            // Reset the entry to 0
            *entry = (struct json__hash_map_entry) {0};
        }
        return true;
    }

    while (entry->next != NULL) {
        if (json__string_eq(entry->next->key, key)) {
            // NOTE: This leaves a unused slot over. It will be removed when we resize the hash map
            json__hash_map_entry_delete(entry->next);
            entry->next = entry->next->next;
            return true;
        }
        entry = entry->next;
    }

    return false;
}

static struct json__hash_map *json__hash_map_copy(struct json__hash_map *hm) {
    struct json__hash_map_entry *bucket = JSON_CALLOC(hm->bucket_cap, sizeof(*bucket));
    if (bucket == NULL) {
        return NULL;
    }
    struct json__hash_map_entry *collisions = JSON_CALLOC(hm->bucket_cap, sizeof(*collisions));
    if (collisions == NULL) {
        JSON_FREE(bucket);
        return NULL;
    }
    struct json__hash_map new_hm = {
        .bucket = bucket,
        .bucket_cap = hm->bucket_cap,
        .bucket_size = hm->bucket_size,

        .collisions = collisions,
        .collisions_cap = hm->bucket_cap,
        .collisions_size = hm->collisions_size,
    };

    bool ok = false;
    for (size_t i = 0; i < hm->bucket_cap; i++) {
        new_hm.bucket[i] = json__hash_map_entry_copy(hm->bucket[i], new_hm.collisions, hm->collisions, &ok);
        if (!ok) {
            json__hm_delete(&new_hm);
            return NULL;
        }
    }

    for (size_t i = 0; i < hm->collisions_size; i++) {
        new_hm.collisions[i] = json__hash_map_entry_copy(hm->collisions[i], new_hm.collisions, hm->collisions, &ok);
        if (!ok) {
            json__hm_delete(&new_hm);
            return NULL;
        }
    }

    struct json__hash_map *ptr = JSON_MALLOC(sizeof(*ptr));
    if (ptr == NULL) {
        json__hm_delete(&new_hm);
        return NULL;
    }
    *ptr = new_hm;
    return ptr;
}

static struct json__hash_map_entry json__hash_map_entry_copy(struct json__hash_map_entry entry, struct json__hash_map_entry *new_collisions, struct json__hash_map_entry *old_collisions, bool *ok) {
    if (!json__hash_map_entry_valid(&entry)) {
        return (struct json__hash_map_entry) {0};
    }

    *ok = true;

    struct json__hash_map_entry new_entry = {0};

    if (entry.next != NULL) {
        // Because next is a pointer, we need to get it's actual index in the collisions array and then add that back to the new collisons array
        new_entry.next = (entry.next - old_collisions) + new_collisions;
    }

    new_entry.key = json_string_copy(entry.key, ok);
    if (!*ok) {
        return new_entry;
    }

    new_entry.value = json_value_copy(entry.value, ok);
    if (!*ok) {
        json_string_delete(new_entry.key);
    }
    return new_entry;
}

static bool json__hash_map_entry_valid(struct json__hash_map_entry *entry) {
    return entry->value.type != JSON_INVALID;
}

#endif // JSON_IMPLEMENTATION

#endif // JSON_H_INC

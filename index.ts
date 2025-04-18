import { readFileSync, writeFileSync } from 'node:fs';
const actualArgs = process.argv.slice(2)
const input = actualArgs[0]
const output = actualArgs[1]

if (!input) {
    console.error("missing first argument for input")
    process.exit(1)
}

if (!output) {
    console.error("missing first argument for output")
    process.exit(1)
}

const content = readFileSync(input).toString("utf8");
const jsonContent = JSON.parse(content)

const arrays: string[] = [];
let header: string = "";
let configMacros = {
    malloc: "malloc",
    realloc: "realloc",
    free: "free",
    assert: "assert"
};
let prefix = "";

for (const key in jsonContent) {
    const content = jsonContent[key]
    if (key === "arrays") {
        if (Array.isArray(content)) {
            for (let i = 0; i < content.length; i++) {
                const arrElement = content[i]
                if (typeof arrElement === "string") {
                    arrays.push(arrElement)
                } else {
                    console.error(`INVALID ELEMENT IN "${key}", expected string, got ${typeof arrElement}`)
                    process.exit(1)
                }
            }
        }
    } else if (key === "header") {
        if (typeof content === "string") {
            header = content;
        } else {
            console.error(`INVALID TYPE FOR "${key}", expected string, got ${typeof content}`)
            process.exit(1)
        }
    } else if (key === "malloc") {
        if (typeof content === "string") {
            configMacros.malloc = content;
        } else {
            console.error(`INVALID TYPE FOR "${key}", expected string, got ${typeof content}`)
            process.exit(1)
        }
    } else if (key === "realloc") {
        if (typeof content === "string") {
            configMacros.realloc = content;
        } else {
            console.error(`INVALID TYPE FOR "${key}", expected string, got ${typeof content}`)
            process.exit(1)
        }
    } else if (key === "free") {
        if (typeof content === "string") {
            configMacros.free = content;
        } else {
            console.error(`INVALID TYPE FOR "${key}", expected string, got ${typeof content}`)
            process.exit(1)
        }
    } else if (key === "prefix") {
        if (typeof content === "string") {
            prefix = content;
        } else {
            console.error(`INVALID TYPE FOR "${key}", expected string, got ${typeof content}`)
            process.exit(1)
        }
    } else if (key === "assert") {
        if (typeof content === "string") {
            configMacros.assert = content;
        } else {
            console.error(`INVALID TYPE FOR "${key}", expected string, got ${typeof content}`)
            process.exit(1)
        }
    } else {
        console.error(`INVALID KEY "${key}"`)
        process.exit(1)
    }
}

let outputText = `
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

// Header Begin
${header}
// Heaer End

enum array_err {
    ARRAY_OK,
    ARRAY_OOM
};

typedef enum array_err array_err;
`

for (let i = 0; i < arrays.length; i++) {
    const e = arrays[i]
    if (!e) {
        throw new Error("Invalid JavaScript Array, should not happen.");
    }
    const niceName = e.replaceAll("*", "_ptr").replaceAll(" ", "_").replaceAll(/_+/g, "_")
    const arrayName = prefix + "array_" + niceName;
    const arrayNameUpperCase = arrayName.toUpperCase();
    const sliceName = prefix + "slice_" + niceName;
    const initialSize = 4;

    outputText +=
        `
// Begin ${e}

struct ${sliceName} {
    ${e} *items;
    size_t len;
};

typedef struct ${sliceName} ${sliceName};

struct ${arrayName} {
    ${e} *items;
    size_t len;
    size_t cap;
};

typedef struct ${arrayName} ${arrayName};

void ${arrayName}_delete(${arrayName} arr) {
    ${configMacros.free}(arr.items);
}

array_err ${arrayName}_grow(${arrayName} *arr) {
    if (arr->items == NULL) {
        arr->items = ${configMacros.malloc}(sizeof(arr->items[0]) * ${initialSize});
        if (arr->items == NULL) {
            return ARRAY_OOM;
        }
        arr->cap = ${initialSize};
    } else {
        size_t new_cap = sizeof(arr->items[0]) * arr->cap * 2;
        ${e}* items = ${configMacros.realloc}(arr->items, new_cap);
        if (arr->items == NULL) {
            return ARRAY_OOM;
        }
        arr->items = items;
        arr->cap = new_cap;
    }
    return ARRAY_OK;
}

array_err ${arrayName}_grow_until(${arrayName} *arr, size_t until) {
    while(arr->cap < until) {
        array_err err = ${arrayName}_grow(arr);
        if (err != ARRAY_OK) {
            return err;
        }
    }
    return ARRAY_OK;
}

array_err ${arrayName}_push(${arrayName} *arr, ${e} item) {
    if (arr->cap <= arr->len) {
        array_err err = ${arrayName}_grow_until(arr, arr->len + 1);
        if (err != ARRAY_OK) {
            return err;
        }
    }
    arr->items[arr->len] = item;
    arr->len += 1;

    return ARRAY_OK;
}

array_err ${arrayName}_append(${arrayName} *arr, ${sliceName} slice) {
    size_t new_size = slice.len + arr->len;
    if (arr->cap <= slice.len + arr->len) {
        array_err err = ${arrayName}_grow_until(arr, new_size);
        if (err != ARRAY_OK) {
            return err;
        }
    }
    for (size_t i = 0; i < slice.len; i++) {
        arr->items[arr->len + i] = slice.items[i];
    }
    arr->len += slice.len;
    return ARRAY_OK;
}

#define ${arrayNameUpperCase}_APPEND(arr, ...) ${arrayName}_append((arr), (${sliceName}){ .items = (${e}[]) { __VA_ARGS__ }, .len = sizeof((${e}[]){ __VA_ARGS__ }[0]) })


void ${arrayName}_unordered_remove(${arrayName} *arr, size_t at) {
    assert(0 <= at && at < arr->len);
    arr->items[at] = arr->items[arr->len - 1];
    arr->len -= 1;
}

void ${arrayName}_ordererd_remove(${arrayName} *arr, size_t at) {
    assert(0 <= at && at < arr->len);
    for (size_t i = at; i < arr->len - 1; i++) {
       	arr->items[i] = arr->items[i+1];
    }
    arr->len -= 1;
}

void ${arrayName}_pop(${arrayName} *arr) {
    assert(arr->len > 0);
    arr->len -= 1;
}

void ${arrayName}_pop_elements(${arrayName} *arr, size_t elements) {
    assert(arr->len > 0);
    arr->len -= elements;
}

${e} ${arrayName}_get(${arrayName} *arr, size_t at) {
    assert(at < arr->len);
    return arr->items[at];
}

${e} ${arrayName}_set(${arrayName} *arr, size_t at, ${e} value) {
    assert(at < arr->len);
    ${e} old_value = arr->items[at];
    arr->items[at] = value;
    return old_value;
}

// IMPORTANT: This slice is not owned, it has the same lifetime as the original array
${sliceName} ${arrayName}_slice(${arrayName} *arr, size_t from, size_t to) {
    assert(0 <= from);
    assert(to <= arr->len);
    assert(to < from);
    return (${sliceName}){
        .items = arr->items + from,
        .len = to,
    };
}

// IMPORTANT: This slice is not owned, it has the same lifetime as the original slice
${sliceName} ${sliceName}_slice(${sliceName} *slice, size_t from, size_t to) {
    assert(0 <= from);
    assert(to <= slice->len);
    assert(to < from);
    return (${sliceName}){
        .items = slice->items + from,
        .len = to,
    };
}

array_err ${arrayName}_to_owned_slice(${arrayName} *arr, ${sliceName} *dst) {
    ${e} *new_slice = ${configMacros.malloc}(sizeof(${e}) * arr->len);

    if (new_slice == NULL) {
        return ARRAY_OOM;
    }

    for (size_t i = 0; i < arr->len; i++) {
        new_slice[i] = arr->items[i];
    }

    *dst = (${sliceName}){
        .items = new_slice,
        .len = arr->len,
    };

    return ARRAY_OK;
}

void ${sliceName}_delete_owned(${sliceName} slice) {
    ${configMacros.free}(slice.items);
}

${e} ${sliceName}_get(${sliceName} *slice, size_t at) {
    assert(at < slice->len);
    return slice->items[at];
}

${e} ${sliceName}_set(${sliceName} *slice, size_t at, ${e} value) {
    assert(at < slice->len);
    ${e} old_value = slice->items[at];
    slice->items[at] = value;
    return old_value;
}

// End ${e}
`
}

writeFileSync(output, outputText)


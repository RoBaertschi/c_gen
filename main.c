#include "test.c"
#include <stdio.h>

int main(void) {
	array_size_t arr = {0};
	ARRAY_SIZE_T_APPEND(&arr, 1, 2, 3, 4, 5, 6, 7, 8);
	for (size_t i = 0; i < arr.len; i++) {
		printf("%zu", array_size_t_get(&arr, i));
	}
	puts("\n");
	array_size_t_ordererd_remove(&arr, 2);
	for (size_t i = 0; i < arr.len; i++) {
		printf("%zu", array_size_t_get(&arr, i));
	}
	puts("\n");
	array_size_t_unordered_remove(&arr, 0);
	for (size_t i = 0; i < arr.len; i++) {
		printf("%zu", array_size_t_get(&arr, i));
	}
	// Should not panic
	array_size_t_get(&arr, 0);
	puts("\n");

	array_size_t_set(&arr, 0, 4);
	for (size_t i = 0; i < arr.len; i++) {
		printf("%zu", array_size_t_get(&arr, i));
	}
	puts("\n");

    slice_size_t slice = {0};
    assert(array_size_t_to_owned_slice(&arr, &slice) == ARRAY_OK);
	for (size_t i = 0; i < slice.len; i++) {
		printf("%zu", slice_size_t_get(&slice, i));
	}
    slice_size_t_delete_owned(slice);
    array_size_t_delete(arr);
}

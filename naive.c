#include <stdio.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef uint32_t u32;

#define print_u32_arr(array, len, name) \
    printf(name": ["); \
    for (int i = 0; i < len; i++) { \
        printf("%d", array[i]); \
        if (i == len - 1) { \
            printf("]\n"); \
        } else { \
            printf(", "); \
        } \
    } \

#define ARR_LEN 18
u32 arr[ARR_LEN] = {};

int main() {
    printf("Hello, World (Naive)\n");

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    for (int i = 0; i < 100000000; i++) {
        for (int j = 0; j < ARR_LEN; j++) {
            arr[j] += 1;
        }
    }

    QueryPerformanceCounter(&end);
    printf("duration: %f\n", (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart);
    print_u32_arr(arr, ARR_LEN, "arr")
}

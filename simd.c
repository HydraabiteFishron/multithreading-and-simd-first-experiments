#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef uint32_t u32;

#define print_u32_arr(array, len, name) \
    printf(""name": ["); \
    for (int i = 0; i < len; i++) { \
        printf("%d", array[i]); \
        if (i == len - 1) { \
            printf("]\n"); \
        } else { \
            printf(", "); \
        } \
    } \

#define VECTOR_WIDTH_BITS 128
#define VECTOR_WIDTH_BYTES (VECTOR_WIDTH_BITS/8)
#define U32_LANES (VECTOR_WIDTH_BYTES / (int)sizeof(u32))

#define ARR_LEN 18
u32 arr[ARR_LEN] = {};
u32 add[U32_LANES] = {};
// u32 res[U32_LANES] = {};

int main() {
    printf("Hello, World (%d)\n", U32_LANES);

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    for (int i = 0; i < ARR_LEN; i++) { arr[i] = 0; }
    for (int i = 0; i < U32_LANES; i++) { add[i] = 1; }

    __m128i vec_b = _mm_loadu_si128((const __m128i*)add);
    for (int i = 0; i < 100000000; i++) {
        int end = 0;
        while (end + U32_LANES <= ARR_LEN) {
            __m128i vec_a = _mm_loadu_si128((const __m128i*)&arr[end]);
            __m128i vec_c = _mm_add_epi32(vec_a, vec_b);
            _mm_storeu_si128((__m128i*)&arr[end], vec_c);

            end += U32_LANES;
        }

        for (int i = end; i < ARR_LEN; i++) {
            arr[i] += 1;
        }
    }

    QueryPerformanceCounter(&end);
    printf("duration: %f\n", (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart);

    print_u32_arr(arr, ARR_LEN, "arr");
    print_u32_arr(add, U32_LANES, "add");
}

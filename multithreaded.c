#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define true 1
#define false 0
#define nil NULL

typedef uint32_t u32;
typedef uint8_t bool;

typedef struct {
    HANDLE handle;
    int index;

    LARGE_INTEGER start, end;
} Thread;

bool thread_init(Thread *thread, unsigned long(*callback)(void *), void *parameters, int index) {
    Thread t = {};
    t.index = index;
    t.handle = CreateThread(
        NULL, 0,
        callback, parameters,
        CREATE_SUSPENDED, NULL
    );

    if (t.handle == nil) {
        return false;
    }

    *thread = t;
    return true;
}

void thread_destroy(Thread *thread) {
    if (thread->handle != nil) {
        CloseHandle(thread->handle);

        Thread empty = {};
        *thread = empty;
    }
}

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

int get_cache_line_size() {
    DWORD buffer_size = 0;

    if (!GetLogicalProcessorInformation(nil, &buffer_size) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(buffer_size);
        if (buffer == NULL) { return 64; }
        
        if (GetLogicalProcessorInformation(buffer, &buffer_size)) {
            DWORD buffer_len = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            
            for (DWORD i = 0; i < buffer_len; i++) {
                if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
                    size_t cache_line_size = buffer[i].Cache.LineSize;
                    free(buffer);
                    return cache_line_size;
                }
            }
        }
        free(buffer);    
    }
    return 64;
}

#define ARR_LEN 18
#define ITERATIONS 100000000

u32 thread_count = 0;
u32 cache_line_size = 0;

Thread *threads = nil;
u32 arr[ARR_LEN] = {};

// just to make sure
void close_all_threads() {
    for (int i = 0; i < thread_count; i++) {
        thread_destroy(&threads[i]);
    }
}

unsigned long worker_callback(void *parameters) {
    Thread *thread = (Thread*)parameters;
    QueryPerformanceCounter(&thread->start);

    int iterations_per_thread = ITERATIONS / thread_count;
    u32 local_arr[18] = {};

    for (int i = 0; i < iterations_per_thread; i++) {
        for (int j = 0; j < ARR_LEN; j++) {
            local_arr[j] += 1;
        }
    }

    for (int i = 0; i < ARR_LEN; i++) {
        arr[i] += local_arr[i];
    }

    QueryPerformanceCounter(&thread->end);
    return 0;
}

int main() {
    SYSTEM_INFO sysinfo = {};
    GetSystemInfo(&sysinfo);

    thread_count = (u32)sysinfo.dwNumberOfProcessors;
    if (thread_count == 0) {
        printf("somethings fishy here\n");
        return 1;
    }

    cache_line_size = get_cache_line_size();
    if (cache_line_size == 0) {
        printf("very fishy\n");
        return 1;
    }

    printf("Hello, Multithreaded (Threads: %d, Cache Line Size: %d)\n", thread_count, cache_line_size);

    // shadjkfhskdjfhakjsgdkj

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    printf("intializing threads\n");
    threads = calloc(thread_count, sizeof(Thread));
    for (int i = 0; i < thread_count; i++) {
        if (!thread_init(&threads[i], worker_callback, &threads[i], i)) {
            printf("failed to initialize a thread (index: %d)\n", i);
            close_all_threads();
            return 1;
        }

        if (threads[i].handle == nil) {
            printf("null handle\n");
            return 1;
        }
    }

    printf("starting threads\n");
    for (int i = 0; i < thread_count; i++) {
        ResumeThread(threads[i].handle);
    }

    printf("awaiting threads\n");
    for (int i = 0; i < thread_count; i++) {
        WaitForSingleObject(threads[i].handle, INFINITE);

        double thread_dur = (double)(threads[i].end.QuadPart - threads[i].start.QuadPart);
        thread_dur /= (double)freq.QuadPart;

        thread_destroy(&threads[i]);
        printf("THREAD %d finished - time elapsed: %f\n", i, thread_dur);
    }

    QueryPerformanceCounter(&end);
    printf("time elapsed: %f\nthreads used: %d\n", (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart, thread_count);

    print_u32_arr(arr, ARR_LEN, "arr");
    close_all_threads();
    printf("end of program\n");
}

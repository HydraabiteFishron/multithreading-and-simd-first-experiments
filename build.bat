@echo off

echo building simd
gcc -msse2 simd.c -o simd

echo building naive
gcc naive.c -o naive

echo building multithreaded
gcc multithreaded.c -o multithreaded

echo building multithreaded simd
gcc -msse2 multithreaded_simd.c -o multithreaded_simd

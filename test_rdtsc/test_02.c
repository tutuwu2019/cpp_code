include <stdio.h>
#include <stdint.h>

uint64_t rdtsc_start() {
    unsigned int low, high;
    __asm__ volatile ("cpuid\n\t"
                      "rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

uint64_t rdtsc_end() {
    unsigned int low, high;
    __asm__ volatile ("rdtscp\n\t"
                      "mov %%eax, %0\n\t"
                      "mov %%edx, %1\n\t"
                      "cpuid"
                      : "=r" (low), "=r" (high)
                      :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)high << 32) | low;
}

int main() {
    uint64_t start, end;
    int a = 1000, b = 200;
    int result;

    start = rdtsc_start();
    
    // Perform division
    result = a / b;

    end = rdtsc_end();

    printf("Division result: %d\n", result);
    printf("CPU cycles for division: %llu\n", end - start);

    return 0;
}

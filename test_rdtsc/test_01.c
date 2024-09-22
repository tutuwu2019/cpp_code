#include <stdio.h>
#include <stdint.h>

uint64_t rdtsc_start() {
    unsigned int low, high;
    __asm__ volatile ("cpuid\n\t"  // Serialize to ensure RDTSC is executed in order
                      "rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

uint64_t rdtsc_end() {
    unsigned int low, high;
    __asm__ volatile ("rdtscp\n\t"  // Ensure serialization after RDTSC
                      "mov %%eax, %0\n\t"
                      "mov %%edx, %1\n\t"
                      "cpuid"
                      : "=r" (low), "=r" (high)
                      :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)high << 32) | low;
}

int main() {
    uint64_t start, end;
    int a = 100, b = 200;
    int result;

    start = rdtsc_start();
    
    // Perform multiplication
    result = a * b;

    end = rdtsc_end();

    printf("Multiplication result: %d\n", result);
    printf("CPU cycles for multiplication: %llu\n", end - start);

    return 0;
}

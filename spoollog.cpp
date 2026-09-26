#define NOMINMAX
#include <Windows.h>
#include <cstdio>

// Simple stub implementations for the debug logging functions
// These are referenced in spoofcall.h but we don't need them for the demo

extern "C" void __stdcall SpoofLogPC(uintptr_t pc, const char* tag) {
    // Uncomment for debugging
    // printf("[SpoofLogPC] %s: 0x%p\n", tag, (void*)pc);
}

extern "C" void __stdcall SpoofLog4(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, const char* tag) {
    // Uncomment for debugging
    // printf("[SpoofLog4] %s: 0x%p 0x%p 0x%p 0x%p\n", tag, (void*)a, (void*)b, (void*)c, (void*)d);
}

extern "C" void __stdcall SpoofLog2(uintptr_t a, uintptr_t b, const char* tag) {
    // Uncomment for debugging
    // printf("[SpoofLog2] %s: 0x%p 0x%p\n", tag, (void*)a, (void*)b);
}
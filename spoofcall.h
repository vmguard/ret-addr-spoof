#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <random>

extern "C" {
    void spoofcall_stub();
    extern uintptr_t proxy_call_returns[32];
    // check spoollog.cpp
    void __stdcall SpoofLogPC(uintptr_t pc, const char* tag);
    void __stdcall SpoofLog4(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, const char* tag);
    void __stdcall SpoofLog2(uintptr_t a, uintptr_t b, const char* tag);
}

namespace SpoofCall {

    inline std::vector<uintptr_t> g_Gadgets;
    inline std::vector<uintptr_t> g_FakeStack;
    inline bool                  g_Initialized = false;

    inline void ScanSection(uintptr_t start, DWORD size,
                            std::vector<uintptr_t>& out)
    {
        if (size < 1) return;
        __try {
            auto* data = (const uint8_t*)start;
            for (DWORD j = 0; j < size - 1; j++) {
                if (data[j] == 0xFF && data[j+1] == 0xE3) {
                    out.push_back(start + j);
                    if (out.size() >= 500) return;
                }
            }
        } __except(1) {}
    }

    inline bool Initialize(uintptr_t moduleBase) {
        if (g_Initialized) return true;

        auto* dos = (IMAGE_DOS_HEADER*)moduleBase;
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;

        auto* nt = (IMAGE_NT_HEADERS*)((uint8_t*)moduleBase + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return false;

        auto* sec = IMAGE_FIRST_SECTION(nt);

        for (int i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
            if (!(sec->Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
            ScanSection(moduleBase + sec->VirtualAddress,
                        sec->Misc.VirtualSize, g_Gadgets);
            if (g_Gadgets.size() >= 500) break;
        }

        if (g_Gadgets.empty()) return false;

        std::mt19937_64 rng(GetTickCount64());
        DWORD imgSize = nt->OptionalHeader.SizeOfImage;
        g_FakeStack.resize(16);
        for (auto& addr : g_FakeStack) {
            addr = (rng() % 2 == 0)
                ? g_Gadgets[rng() % g_Gadgets.size()]
                : moduleBase + (rng() % imgSize);
        }

        for (int i = 0; i < 32; i++)
            proxy_call_returns[i] = g_Gadgets[i % g_Gadgets.size()];

        g_Initialized = true;
        return true;
    }

    // funcPtr passed as FIRST arg (RCX), MAGIC as second (RDX).
    // user args start at R8 -> asm remaps R8->RCX, R9->RDX.
    template<typename Ret, typename... Args>
    inline Ret Call(void* func, Args... args) {
        if (!g_Initialized || g_Gadgets.empty()) {
            auto fn = reinterpret_cast<Ret(__fastcall*)(Args...)>(func);
            return fn(args...);
        }
        constexpr uintptr_t MAGIC = 0x52A3450;
        auto stub = reinterpret_cast<Ret(__fastcall*)(void*, uintptr_t, Args...)>(spoofcall_stub);
        return stub(func, MAGIC, args...);
    }
}

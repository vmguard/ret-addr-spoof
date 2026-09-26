#define NOMINMAX
#include "spoofcall.h"
#include <iostream>
#include <Windows.h>

int __fastcall TestFunction(int a, int b) {
    std::cout << "TestFunction called with a=" << a << ", b=" << b << std::endl;
    return a + b;
}

void __fastcall VoidFunction(const char* message) {
    std::cout << "VoidFunction: " << message << std::endl;
}

int main() {
    std::cout << "Spoofcall Demo App\n";

    HMODULE selfModule = GetModuleHandle(nullptr);
    if (!selfModule) {
        std::cerr << "Failed to get module handle\n";
        return 1;
    }

    std::cout << "Module base: 0x" << std::hex << (uintptr_t)selfModule << std::dec << "\n";

    if (!SpoofCall::Initialize((uintptr_t)selfModule)) {
        std::cout << "Failed to initialize spoofcall - no gadgets found\n";
        std::cout << "Will fall back to direct calls\n";
    } else {
        std::cout << "Spoofcall initialized with " << SpoofCall::g_Gadgets.size() << " gadgets\n";
    }

    std::cout << "\nTesting function calls:\n";

    std::cout << "\n1. Direct call to TestFunction:\n";
    int directResult = TestFunction(10, 20);
    std::cout << "   Result: " << directResult << "\n";

    std::cout << "\n2. Call through spoofcall:\n";
    int spoofResult = SpoofCall::Call<int>(TestFunction, 10, 20);
    std::cout << "   Result: " << spoofResult << "\n";


    std::cout << "\n3. Void function through spoofcall:\n";
    SpoofCall::Call<void>(VoidFunction, "Hello from spoofcall!");

    std::cout << "\n4. Testing with larger numbers:\n";
    int bigResult = SpoofCall::Call<int>(TestFunction, 100, 200);
    std::cout << "   Result: " << bigResult << "\n";

    if (!SpoofCall::g_Gadgets.empty()) {
        std::cout << "\nFirst few gadget addresses found:\n";
        size_t count = SpoofCall::g_Gadgets.size();
        if (count > 5) count = 5;
        for (size_t i = 0; i < count; i++) {
            std::cout << "  [" << i << "] 0x" << std::hex << SpoofCall::g_Gadgets[i] << std::dec << "\n";
        }
    }

    std::cout << "\nBye bye!\n";
    std::cin.get();
    return 0;
}
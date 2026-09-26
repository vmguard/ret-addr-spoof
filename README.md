# ret-addr-spoof

## What this does?

Basically messing with how functions return in x64 assembly. The idea is to intercept where a function thinks it should return to, and make it go somewhere else instead. 

There's an asm stub that does the heavy lifting, and a C++ wrapper that makes it somewhat usable. It looks for `jmp rbx` gadgets in a target module's .text section and uses those to bounce execution back to the real return address.

## Files

- `spoofcall.asm` - The assembly stub that does the actual return address manipulation
- `spoofcall.h` - C++ wrapper with some scanning logic to find gadgets

## How it works

1. Scan a target module for `FF E3` (jmp rbx) instructions
2. Set up RBX with the real return address
3. Overwrite the return slot with a gadget address  
4. Tail call to the target function
5. When it returns, it hits the gadget which jumps through RBX back to the real caller

The ASM file has some comments that explain it better than I can here.

## Disclaimer

This is just experimental code, it is most definitely NOT production ready or particularly safe. More of testing a cool idea kind of thing than something you'd actually use.

If you're poking around here, you probably know what you're doing, and probably better than me at programming.

The demo shows:
- Initializing the spoofcall system with your own executable
- Finding `jmp rbx` gadgets in the .text section
- Calling functions through the spoofcall mechanism
- Comparing results with direct calls

It's mostly just to show that the mechanism works at all, nothing fancy.

## Porque?

I have been writing A LOT of asm lately, and I saw a youtube video where a guy made a return address spoofer in asm and C++ so I was like no way I could probably do that too. So yeah, cool PoC, not very unique, interesting way to do it though.

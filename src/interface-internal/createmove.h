#pragma once

#include "../core/core.h"
#include "../core/modules/aimassist.h"
#include <cstdint>
#include "vmt_hook.h"
#include <sys/mman.h>

#include <chrono>

#define VTABLE_IDX 4

inline vmt_hook *hook;

struct CUserCmd {
        int command_number; // 0x0000
        int tick_count;     // 0x0004
        float command_time; // 0x0008
        vec3 viewangles;    // 0x000C
        vec3 delta;
        char pad_0x0018[0x8]; // 0x0018
        float forwardmove;    // 0x002C
        float sidemove;       // 0x0030
        float upmove;         // 0x0034
        int buttons;          // 0x0038
        char impulse;         // 0x003C
};

inline uintptr_t hkCreateMove(void *a1, int a2, float a3, bool a4) {
    /*void *thisptr;
    int sequence_number;
    float unk;
    bool unk1;

    // Hooking on Proton sure is fun
    asm("mov %%rcx, %0\n"
        "mov %%edx, %1\n"
        "movss %%xmm2, %2\n"
        "movb %%r9b, %3\n"
        : [thisptr] "=r"(thisptr), [sequence_number] "=r"(sequence_number), [unk] "=x"(unk), [bUnk1] "=r"(unk1)
        :
        :);
    
    std::cout << "HI" << std::endl;
    
    //AimAssist::before_createmove();
     
    void* function = hook->get_original_method(4);

    int64_t result;
    asm("movq %1, %%rcx\n"
        "movl %2, %%edx\n"
        "movss %3, %%xmm2\n"
        "movb %4, %%r9b\n"
        "call *%5\n"
        "movq %%rax, %0\n"
        : "=r"(result)
        : "r"(thisptr), "r"(sequence_number), "m"(unk), "r"(unk1), "r"(function)
        : "%rcx", "%edx", "%xmm2", "%r9", "%rax");

    //AimAssist::after_createmove();

    return result;*/
}

__attribute__((naked)) bool _1337() {
    __asm__ __volatile__ ("mov QWORD PTR [rsp+0x8], rbx");
    __asm__ __volatile__ ("push rdi");
    __asm__ __volatile__ ("sub rsp, 0x250");
    __asm__ __volatile__ ("mov r11, rcx");
    __asm__ __volatile__ ("movsxd r10, edx");
    __asm__ __volatile__ ("lea rcx, [rsp+0x20]");
    __asm__ __volatile__ ("mov rdi, r8");
    __asm__ __volatile__ ("mov rax, 0x1407B2210");
    __asm__ __volatile__ ("call rax");
    __asm__ __volatile__ ("xor ebx, ebx");
    __asm__ __volatile__ ("cmp r9d, 0xFFFFFFFF");
    __asm__ __volatile__ ("jnz label1");
    __asm__ __volatile__ ("lea r8, [rsp+0x20]");
    __asm__ __volatile__ ("label1:");
    __asm__ __volatile__ ("cmp r10d, 0xFFFFFFFF");
    __asm__ __volatile__ ("jnz label2");
    __asm__ __volatile__ ("mov r8, r11");
    __asm__ __volatile__ ("jmp label3");
    __asm__ __volatile__ ("label2:");
    __asm__ __volatile__ ("imul r8, r10, 0xE40");
    __asm__ __volatile__ ("add r8, r11");
    __asm__ __volatile__ ("label3:");
    __asm__ __volatile__ ("mov eax, 0x57619F1");
    __asm__ __volatile__ ("mov ecx, r9d");
    __asm__ __volatile__ ("imul r9d");
    __asm__ __volatile__ ("sar edx, 4");
    __asm__ __volatile__ ("mov eax, edx");
    __asm__ __volatile__ ("shr eax, 0x1F");
    __asm__ __volatile__ ("add edx, eax");
    __asm__ __volatile__ ("imul eax, edx, 0x2EE");
    __asm__ __volatile__ ("sub ecx, eax");
    __asm__ __volatile__ ("movsxd rax, ecx");
    __asm__ __volatile__ ("imul rdx, rax, 0x228");
    __asm__ __volatile__ ("mov rax, rbx");
    __asm__ __volatile__ ("add rdx, [r8+0xF8]");
    __asm__ __volatile__ ("lea r8, [rsp+0x20]");
    __asm__ __volatile__ ("cmp [rdx], r9d");
    __asm__ __volatile__ ("cmovz rax, rdx");
    __asm__ __volatile__ ("test rax, rax");
    __asm__ __volatile__ ("cmovnz r8, rax");
    __asm__ __volatile__ ("label4:");
    __asm__ __volatile__ ("cmp r10d, 0xFFFFFFFF");
    __asm__ __volatile__ ("jz label5");
    __asm__ __volatile__ ("imul rdx, r10, 0xE40");
    __asm__ __volatile__ ("add r11, rdx");
    __asm__ __volatile__ ("label5:");
    __asm__ __volatile__ ("mov r9d, [rsp+0x280]");
    __asm__ __volatile__ ("mov eax, 0x57619F1");
    __asm__ __volatile__ ("imul r9d");
    __asm__ __volatile__ ("mov ecx, r9d");
    __asm__ __volatile__ ("sar edx, 4");
    __asm__ __volatile__ ("mov eax, edx");
    __asm__ __volatile__ ("shr eax, 0x1F");
    __asm__ __volatile__ ("add edx, eax");
    __asm__ __volatile__ ("imul eax, edx, 0x2EE");
    __asm__ __volatile__ ("sub ecx, eax");
    __asm__ __volatile__ ("movsxd rax, ecx");
    __asm__ __volatile__ ("mov rcx, rdi");
    __asm__ __volatile__ ("imul rdx, rax, 0x228");
    __asm__ __volatile__ ("add rdx, [r11+0xF8]");
    __asm__ __volatile__ ("cmp [rdx], r9d");
    __asm__ __volatile__ ("cmovz rbx, rdx");
    __asm__ __volatile__ ("lea rdx, [rsp+0x20]");
    __asm__ __volatile__ ("test rbx, rbx");
    __asm__ __volatile__ ("cmovnz rdx, rbx");
    __asm__ __volatile__ ("movabs rax, 0x1407B24B0");
    __asm__ __volatile__ ("call rax");
    __asm__ __volatile__ ("cmp BYTE PTR [rdi+0x14], 0");
    __asm__ __volatile__ ("mov rbx, [rsp+0x20]");
    __asm__ __volatile__ ("setz al");
    __asm__ __volatile__ ("add rsp, 0x250");
    __asm__ __volatile__ ("pop rdi");
    __asm__ __volatile__ ("ret");
} // Todo: make this work.

inline void func_copy(uintptr_t address, char* func, size_t length) {
    mprotect((void*)(address & ~0xFFF), length, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy((void*)address, func, length);
    mprotect((void*)(address & ~0xFFF), length, PROT_READ | PROT_EXEC);
}

inline void init() {
    uintptr_t addr = 0x14efbc000;
    func_copy(addr, (char*)&_1337, 0x1000); // Crash
    uintptr_t cinput = 0x1420a9960;
    uintptr_t vtable_idx6 = (cinput + 0x8 * 6); // CInput::WriteUsercmdDeltaToBuffer
    *(uintptr_t*)vtable_idx6 = addr;
}
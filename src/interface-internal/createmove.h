#pragma once

#include "../core/core.h"
#include <cstdint>
#include "vmt_hook.h"

#include <chrono>

#define VTABLE_IDX 4

inline vmt_hook *hook;

inline uintptr_t hkCreateMove(void *a1, int a2, float a3, bool a4) {
    void *thisptr;
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

    return result;
}

inline void init() {
    //hook = new vmt_hook();
    //hook->load((void*)(BASE + 0x020aa920), 78);
    //hook->hook_method(VTABLE_IDX, (void*)&hkCreateMove);
    //hook->apply();
}
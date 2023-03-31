#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <cstring>

class vmt_hook
{
private:
    uintptr_t* m_vtable;
    uintptr_t** m_class_ptr;
    std::size_t m_method_count;
    std::unique_ptr<uintptr_t[]> m_new_vtable;
    void* m_instance;

public:
    vmt_hook();

    void load(void* instance, size_t method_count);
    
    void hook_method(std::size_t method_index, void* new_method);

    void apply();

    void* get_original_method(std::size_t method_index);

    inline static uintptr_t get_vtable_method(void* instance, size_t index) {
        auto m_class_ptr = reinterpret_cast<std::uintptr_t**>( instance );
        auto m_vtable = *m_class_ptr;
        return m_vtable[index];
    }
};
#include "vmt_hook.hpp"

vmt_hook::vmt_hook() {}

void vmt_hook::load(void *instance, size_t method_count) {
    m_new_vtable = std::make_unique<std::uintptr_t[]>(method_count);
    m_method_count = method_count;

    m_class_ptr = reinterpret_cast<std::uintptr_t **>(instance);
    m_vtable = *m_class_ptr;

    m_instance = instance;

    std::memcpy(m_new_vtable.get(), m_vtable,
                m_method_count * sizeof(uintptr_t));
}

void vmt_hook::hook_method(std::size_t method_index, void *new_method) {
    m_new_vtable[method_index] = reinterpret_cast<uintptr_t>(new_method);
}

void vmt_hook::apply() {
    *reinterpret_cast<uintptr_t **>(m_instance) = m_new_vtable.get();
}

void *vmt_hook::get_original_method(std::size_t method_index) {

    return reinterpret_cast<uintptr_t *>(m_vtable[method_index]);
}
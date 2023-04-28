#pragma once

#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <string>
#include <vector>

inline constexpr uintptr_t base = 0x140000000;
namespace mem {
    inline pid_t pid = 0;
    
    template <typename T>
    T read(uintptr_t address) {
        T buffer;
        struct iovec local[1], remote[1];
        local->iov_base = &buffer;
        local->iov_len = sizeof(T);
        remote->iov_base = reinterpret_cast<void*>(address);
        remote->iov_len = local->iov_len;
        process_vm_readv(pid, local, 1, remote, 1, 0);
        return buffer;
    }

    template <typename T>
    void write(uintptr_t address, T value) {
        struct iovec local[1], remote[1];
        local->iov_base = &value;
        local->iov_len = sizeof(T);
        remote->iov_base = reinterpret_cast<void*>(address);
        remote->iov_len = local->iov_len;
        process_vm_writev(pid, local, 1, remote, 1, 0);
    }

    template <typename T>
    T read(uintptr_t address, size_t len) {
        T buffer;
        struct iovec local[1], remote[1];
        local->iov_base = &buffer;
        local->iov_len = len;
        remote->iov_base = reinterpret_cast<void*>(address);
        remote->iov_len = local->iov_len;
        process_vm_readv(pid, local, 1, remote, 1, 0);
    }

    inline std::string read_string(uintptr_t address, size_t length) {
        std::string result;
        for (size_t i = 0; i < length; i++) {
            char c = mem::read<char>(reinterpret_cast<uintptr_t>(address) + i);
            if (c == '\0') break;
            result += c;
        }
        return result;
    }

    template <typename T>
    inline bool read_array(uintptr_t address, void* buffer, size_t size) {
        iovec local[1];
        iovec remote[1];

        local[0].iov_base = buffer;
        local[0].iov_len = size * sizeof(T);
        remote[0].iov_base = reinterpret_cast<void*>(address);
        remote[0].iov_len = size * sizeof(T);

        ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
        if (nread != size * sizeof(T)) {
            return false;
        }

        return true;
    }

}
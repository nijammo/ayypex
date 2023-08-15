```cpp
#ifndef MEM_H
#define MEM_H

namespace core {

class mem {
public:
    static void read(void* destination, const void* source, size_t size) {
        memcpy(destination, source, size);
    }

    static void write(void* destination, const void* source, size_t size) {
        memcpy(destination, source, size);
    }
};

} // namespace core

#endif // MEM_H
```
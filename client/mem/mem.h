
#ifndef CLIENT_MEM_MEM_H
#define CLIENT_MEM_MEM_H

#include <cstdint>

class Mem {
public:
    template <typename T>
    static void setValue(uintptr_t, T);

    template <typename T>
    static T getValue(uintptr_t);

    static auto findSig(const char*, const char* modulename = "Minecraft.Windows.exe") -> uintptr_t;

    // 在地址后进行特征查找直到超出规定范围
    static auto findSigRelay(uintptr_t, const char*, size_t rang)->uintptr_t;
};

template<typename T>
void Mem::setValue(uintptr_t ptr, T v) {
    *reinterpret_cast<T*>(ptr) = v;
}

template<typename T>
T Mem::getValue(uintptr_t ptr) {
    return *reinterpret_cast<T*>(ptr);
}

#endif //CLIENT_MEM_MEM_H
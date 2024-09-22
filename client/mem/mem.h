
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
    static auto findSigRelay(uintptr_t, const char*, size_t rang) -> uintptr_t;

    /**
     * @brief 可在一个函数的调用者处定位这个函数
     * @param sig 特征码
     * @param offset 特征码开头到call偏移之前的偏移
     * @return 0 失败 否则 call地址
     */
    static auto funcFromSigOffset(uintptr_t sig, int offset)->uintptr_t;

    template < typename ret>
    static auto funcFromSigOffset(uintptr_t sig, int offset) -> ret;

    static auto getBase(const char* module = "Minecraft.Windows.exe") -> uintptr_t;
};

template<typename T>
void Mem::setValue(uintptr_t ptr, T v) {
    *reinterpret_cast<T*>(ptr) = v;
}

template<typename T>
T Mem::getValue(uintptr_t ptr) {
    return *reinterpret_cast<T*>(ptr);
}

template < typename ret>
static auto funcFromSigOffset(uintptr_t sig, int offset) -> ret {
    auto jmpval = *reinterpret_cast<int*>(sig + offset);
    return reinterpret_cast<ret>(sig + offset + 4 + jmpval);
}

#endif //CLIENT_MEM_MEM_H
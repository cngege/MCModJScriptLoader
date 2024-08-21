
#ifndef CLIENT_MEM_MEM_H
#define CLIENT_MEM_MEM_H

#include <cstdint>

#define INRANGE(x,a,b) (x >= a && x <= b)
#define GET_BYTE( x )  (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )  (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))

class Mem {
public:
    static auto findSig(const char*, const char* modulename = "Minecraft.Windows.exe") -> uintptr_t;

    // 在地址后进行特征查找直到超出规定范围
    static auto findSigRelay(uintptr_t, const char*, size_t rang)->uintptr_t;
};


#endif //CLIENT_MEM_MEM_H
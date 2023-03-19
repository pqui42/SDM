
#include "bin.h"
#include <cassert>

//typedef AlignmentRequirements AR;

std::size_t diff(void *p1, void *p0) {
    return reinterpret_cast<std::size_t>(p1) - reinterpret_cast<std::size_t>(p0); 
}


template <typename T>
void test_alignment(){
    std::cout << PoolAccess<T>::Get().addressof(0) << std::endl;
    std::cout << PoolAccess<T>::Get().addressof(1) << std::endl; 
    assert(diff(PoolAccess<T>::Get().addressof(1), PoolAccess<T>::Get().addressof(0))  == alignof(T));
} 

int main() {

    MemoryPoolCreator(/*cfg*/);

    PoolAccess<char>::Get()[0] = 'h';
    PoolAccess<char>::Get()[1] = 'i';
    PoolAccess<char>::Get()[3] = 'i';

    assert(PoolAccess<char>::Get()[0] == 'h');
    assert(PoolAccess<char>::Get()[1] == 'i');

    test_alignment<uint64_t>();
    test_alignment<uint32_t>();
    test_alignment<uint16_t>();
    test_alignment<uint8_t>();
}

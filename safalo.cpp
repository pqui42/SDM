
#include <iostream>
#include <cstddef>

#include "safalo.h"


void * operator new(std::size_t size) {
    if(!SafAlo::Get().IsAloAllowed()) {
        std::cerr << "Allocation Prohibited\n";
        std::abort();
    }
    if (size == 0) {
        size = 1;
    }
    void* p = nullptr;
    if ((p = ::malloc(size)) == nullptr)
    {
        std::cerr << "bad alloc \n";
        std::abort();
    }

    #ifdef DEBUG_SAFALO
        std::cout << __FUNCTION__ << " ret : " << p << std::endl;
    #endif 
    return p;
}


void* operator new(size_t size, const std::nothrow_t&) noexcept {
    void* p = nullptr;
    p = ::operator new(size);
    return p;
}

void* operator new[](size_t size) {
    return ::operator new(size);
}


void* operator new[](size_t size, const std::nothrow_t&) noexcept {
    void* p = nullptr;
    p = ::operator new[](size);
    return p;
}


void operator delete(void* ptr) noexcept {
    ::free(ptr);
    #ifdef DEBUG_SAF_ALO
        std::cout << __FUNCTION__ << " free @" << ptr << std::endl;
    #endif 
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    ::operator delete(ptr);
}


void operator delete(void* ptr, size_t) noexcept {
    ::operator delete(ptr);
}

void operator delete[] (void* ptr) noexcept {
    ::operator delete(ptr);
}

void operator delete[] (void* ptr, const std::nothrow_t&) noexcept {
    ::operator delete[](ptr);
}

void operator delete[] (void* ptr, size_t) noexcept
{
    ::operator delete[](ptr);
}
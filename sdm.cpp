#include <iostream>
#include <thread>
#include "sdm.h"

void * operator new(std::size_t size)
{
    void* p = nullptr;
    if(DynamicMemory::getInstance().IsRequestToOS()) {
        if (size == 0) {
            size = 1;
        }
        while ((p = ::malloc(size)) == nullptr)
        {
            // If malloc fails and there is a new_handler,
            // call it to try free up memory.
            std::new_handler nh = std::get_new_handler();
            if (nh) {
                    nh();
            }    else {
        #ifndef NO_EXCEPTIONS
                    throw std::bad_alloc();
        #else
                    break;
        #endif
            }
        }
       
    } else {  
        #ifndef SAFE_MALLOC
            std::cerr << "SAFE_MALLOC not defined \n";
            std::abort();
        #else
            p = ::safe_malloc(size); // safe_malloc needs to be implemented
            if (!p) {
                    std::abort();
            }
        #endif
    }
    return p;
}


void* operator new[](size_t size)
{
    return ::operator new(size);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
#ifndef NO_EXCEPTIONS
    try
    {
#endif // NO_EXCEPTIONS
        p = ::operator new(size);
#ifndef NO_EXCEPTIONS
    }
    catch (...)
    {
    }
#endif // NO_EXCEPTIONS
    return p;
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
#ifndef NO_EXCEPTIONS
    try
    {
#endif // NO_EXCEPTIONS
        p = ::operator new[](size);
#ifndef NO_EXCEPTIONS
    }
    catch (...)
    {
    }
#endif // NO_EXCEPTIONS
    return p;
}


void operator delete(void* ptr) noexcept
{
    if(DynamicMemory::getInstance().IsRequestToOS()) {
        ::free(ptr);
    } else {
        #ifndef SAFE_MALLOC
            std::abort();
        #else
            ::safe_free(ptr); // safe_free needs to be implemented
        #endif
    }
}


void  operator delete[](void* ptr) noexcept {
    ::operator delete(ptr);
}

void  operator delete[](void* ptr, std::size_t /*size*/) noexcept {
        ::operator delete[](ptr);
}

void  operator delete(void* ptr, std::size_t /*size*/) noexcept {
    ::operator delete(ptr);
}
void  operator delete(void* ptr, const std::nothrow_t&) noexcept{
    ::operator delete(ptr);
}

void  operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    ::operator delete[](ptr);
}
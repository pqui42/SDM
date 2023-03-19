#ifndef TYPE_NAME_H
#define TYPE_NAME_H

#include <string>
#include <cxxabi.h>

/// @brief GetTypeName
/// @tparam T 
/// @return returns the demangled name of T
template <class T>
std::string GetTypeName() {
    auto m_name = typeid(T).name();
    
    size_t buff_size = sizeof(m_name);

    char* buff  = reinterpret_cast<char*>(std::malloc(buff_size));;
    int stat = 0;
    if(buff) {
        buff = abi::__cxa_demangle(m_name, buff, &buff_size, &stat);
    } else {
        std::abort();
    }
    std::string ret(buff);
    std::free(buff);
    return ret;
}


#endif // TYPE_NAME_H
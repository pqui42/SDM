
// [1] https://en.cppreference.com/w/cpp/memory/new/operator_new 
// 
// definitions (1) (2) (5) (6) are relevant for c++14 and replacable see [1]
// all other definitions are non-allocating, user defined or class specific  

// void* operator new(std::size_t size);                                   // replaceable, nodiscard in C++20
// void* operator new[](std::size_t size);                                 // replaceable, nodiscard in C++20
// void* operator new(std::size_t size, const std::nothrow_t&) noexcept;   // replaceable, nodiscard in C++20
// void* operator new[](std::size_t size, const std::nothrow_t&) noexcept; // replaceable, nodiscard in C++20


// [2] https://en.cppreference.com/w/cpp/memory/new/operator_delete
// 
// definitions (1) (2) (5) (6) (9) (10) are relevat for c++14 and replaceable see [2]
// all other definitions are non-allocating, user defined or class specific  

// void  operator delete(void* ptr) noexcept;                              // replaceable
// void  operator delete[](void* ptr) noexcept;                            // replaceable
// void  operator delete[](void* ptr, std::size_t size) noexcept;          // replaceable, C++14
// void  operator delete(void* ptr, std::size_t size) noexcept;            // replaceable, C++14
// void  operator delete(void* ptr, const std::nothrow_t&) noexcept;       // replaceable
// void  operator delete[](void* ptr, const std::nothrow_t&) noexcept;     // replaceable

#ifndef SDM_H
#define SDM_H

#include <mutex>
// Meyers Singleton is automatically thread-safe, since static variables will block scope 
class DynamicMemory {
public:
  static DynamicMemory & getInstance(){
    static DynamicMemory instance;
    return instance;
  }
  void UsePreallocatedDynamicMemory(){
    std::lock_guard<std::mutex> lk(request_to_os_mutex_);
    request_to_os_ = false;
  }

  void RequestFromOS(){
    std::lock_guard<std::mutex> lk(request_to_os_mutex_);
    request_to_os_ = true;
  }

  bool IsRequestToOS() {
    return request_to_os_;
  }

private:
  DynamicMemory() = default;
  ~DynamicMemory() = default;
  DynamicMemory(const DynamicMemory&)= delete;
  DynamicMemory& operator=(const DynamicMemory&)= delete;
  bool request_to_os_ = true;
  mutable std::mutex request_to_os_mutex_; // The "M&M rule": mutable and mutex go together
};

#endif // SDM_H
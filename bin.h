#ifndef BIN_H
#define BIN_H

#define NAME_DEBUG

#include <cstdint>
#include <cstdlib>
#include <new>
#ifdef NAME_DEBUG
#include <iostream>
#include "typename.h"
#endif

/// @brief pool of slots 
class Pool {
    public:
        /// @brief 
        /// @param alignment_req 
        /// @param no_slots 
        Pool(std::size_t alignment_req, std::size_t no_slots) : alignment_req_{alignment_req}, no_slots_{no_slots} { }
        ~Pool() = default;
        Pool(const Pool &) = delete;
        Pool(Pool &&) = delete;
      
        /// @brief pool size in bytes
        /// @return 
        std::size_t GetPoolSize() const {
            return alignment_req_*no_slots_;
        }
        
        /// @brief 
        /// @return 
        std::size_t GetNoSlots() const {
            return no_slots_;
        }
        /// @brief 
        /// @return 
        std::size_t GetAlignmentRequirements() const {
            return alignment_req_;
        }

    private:
        // https://en.cppreference.com/w/cpp/language/object#Alignment
        // Every object type has the property called alignment requirement, 
        // which is a nonnegative integer value (of type std::size_t, and always a power of two) 
        // representing the number of bytes between successive addresses at which objects of this 
        // type can be allocated. 
        const std::size_t alignment_req_;
        const std::size_t no_slots_;
};

class DynamicPool {
    public:
        DynamicPool(std::size_t alignment, std::size_t no_slots) : pool_(alignment, no_slots) {
            p_ = aligned_alloc(pool_.GetAlignmentRequirements(), pool_.GetPoolSize());
            if(!p_) {
                std::abort();
            }
        }

        ~DynamicPool() {
            free(p_);
        }

        DynamicPool(const DynamicPool &) = delete;
        DynamicPool(DynamicPool &&) = delete;
   
        void * PoolBegin() {
            return p_;
        }

    private:
        Pool pool_;
        void * p_;
};

struct AlignmentRequirements {
    static constexpr std::size_t aof1{1};
    static constexpr std::size_t aof2{2};
    static constexpr std::size_t aof4{4};
    static constexpr std::size_t aof8{8};
    static constexpr std::size_t aof16{16};
    static constexpr std::size_t aof32{32};
    static constexpr std::size_t aof64{64};
    static constexpr std::size_t aof128{128};
    static constexpr std::size_t aof256{256};
    static constexpr std::size_t aof512{512};
    static constexpr std::size_t aof1024{1024};
};

// https://riptutorial.com/cplusplus/example/18082/check-if-an-integer-is-a-power-of-2
constexpr bool is_power_of_two(std::size_t n) {
    return n && !(n & (n - 1));
}

template <std::size_t Alignment>
class PoolFactory {
    static_assert(is_power_of_two(Alignment), "alignment has to be a power of 2!");
    public:
        /// @brief pools cannot be recreated. First call creates the pool.
        /// @param no_slots 
        /// @return 
        static PoolFactory& CreatePool(std::size_t no_slots = 1) {
            static PoolFactory pool(Alignment, no_slots);
            return pool;
        }

        void * PoolBegin() {
            return dpool_.PoolBegin();
        }

    private:
        PoolFactory(std::size_t alignment, std::size_t no_slots) : dpool_{alignment, no_slots} {
#ifdef NAME_DEBUG
                std::cout << __FUNCTION__ << "( " <<  "alignment=" << alignment << ", no_slots="<< no_slots << " )\n";
#endif
        }
        ~PoolFactory() {
#ifdef NAME_DEBUG
                std::cout << __FUNCTION__ << "( " <<  "alignment=" << Alignment << " )\n";
#endif
        }
        DynamicPool dpool_;
};
 
void MemoryPoolCreator(/*cfg*/) {
    PoolFactory<AlignmentRequirements::aof1>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof2>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof4>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof8>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof16>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof32>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof64>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof128>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof256>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof512>::CreatePool(100);
    PoolFactory<AlignmentRequirements::aof1024>::CreatePool(100);
}



template <typename T>
class PoolAccess {
    static_assert(is_power_of_two(alignof(T)), "Alignment has to be a power of 2!");
    
    public:

        static PoolAccess& Get() {
            static PoolAccess pa;
            return pa;
        }

        T& operator[](std::size_t n) { return p_[n]; }
        T operator[](std::size_t n) const { return p_[n]; }

        void * addressof(std::size_t n) {
            return static_cast<void*>(std::addressof(p_[n]));
        }

    private:
        PoolAccess() : p_{static_cast<T*>(PoolFactory<alignof(T)>::CreatePool().PoolBegin())} {
#ifdef NAME_DEBUG
        std::cout << __FUNCTION__ << "<" << type_name_ << ">" << "\n";
#endif    
        }

        ~PoolAccess() {
#ifdef NAME_DEBUG
        std::cout << __FUNCTION__ << "<" << type_name_ << ">" << "\n";
#endif           
        } 

        T * p_;
#ifdef NAME_DEBUG
       const std::string type_name_{GetTypeName<T>()};
#endif



};

#endif // BIN_H
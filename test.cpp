
#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <type_traits>
#include <typeinfo>
#include <cxxabi.h>
#include <string_view>

/// @brief GetTypeName
/// @tparam T 
/// @return returns the demangled name of T
template <class T>
std::string_view GetTypeName() {
    return std::string_view(abi::__cxa_demangle(typeid(T).name(), nullptr,
                                           nullptr, nullptr));
}

/// @brief PoolAllocator 
///         
/// @details
///  
///        pool : [slot][slot][slot][slot][slot][slot]
///
///        slot : [pointer to next free slot] or [T] 
///               Note: the last slot contains nullptr to mark the end of the pool.
///  
///        This allocator can only hand out one slot or all slots per allocation.
///        Only one slot or all slots can be handed back to the pool per
///        deallocation.
///                
///        Rationale: This allocator is to be used in conjunction with 
///                   std::map and std::vector. These containers serve the usecase 
///                   to work with a per element approach (see interface of std::map
///                   and std::vector). If this allocation strategie fits into your
///                   objects allocation demangs you are free to use it.  
/// 
///                
///        Dynamic memory, is a global resource. A object requests heap memory on initialisation 
///        and releases it on destruction. To keep the global character a thread safe singelton 
///        pattern is followed, allowing lazy initialization (Mayer's Singelton).
/// 
/// @attention 
/// 
///        Only one PoolAllocator<T> instance will exsit in your programm and once 
///        createatd it will exsit during the entire programms duration. 
///
/// @tparam T 
template <typename T>
class PoolAllocator {
    public:
    /// @brief Getter to the instance of PoolAllocator<T>
    /// @attention never assign the returned instance to a variable. 
    /// @return never use the return statement use PoolAllocator<T>::Get().Function() instead
    static PoolAllocator & Get() {
        static PoolAllocator instance;
        return instance;
    }

    /// @brief Creates a memory pool
    /// @details   The global new operator is used to request memory for the pool.
    ///            no_slots slots are created.
    /// @attention Recreation of the memory pool will abort your programm
    /// @param no_slots 
    void CreateMemoryPool(size_t no_slots) {

        std::cout << __FUNCTION__ << " for type " << GetTypeName<T>() << std::endl;
        
        if(is_pool_created_) {
            std::cout << "Pool for "<< GetTypeName<T>() << " already created" << std::endl;
            std::abort();
        } else {
            no_slots_ = no_slots;
            Slot * pool_begin = CreateMemoryPoolImpl(no_slots);
            alloc_ptr_ = pool_begin;
            pool_begin_ = pool_begin;
        }

        std::cout << __FUNCTION__ << " for type " << GetTypeName<T>() << " done" << std::endl;
    }

    /// @brief Releases the memory pool
    /// @details Uses the global delete operator to release the memory 
    ///          used for the pool
    /// @attention calling this function before CreateMemoryPool or twice in a row
    ///            will abort your programm 
    void ReleaseMemoryPool() {
        std::cout << __FUNCTION__ << " for type " << GetTypeName<T>() << std::endl;
        if(pool_begin_ == nullptr) {
            std::cout << "Pool for "<< GetTypeName<T>() << " already created" << std::endl;
            std::abort();
        }
        ::operator delete(static_cast<void *>(pool_begin_));
        pool_begin_ = nullptr;
        no_slots_ = 0;
        is_pool_created_ = false;
        alloc_ptr_ = nullptr;
        slot_index_ = 0; 
        std::cout << __FUNCTION__ << " for type " << GetTypeName<T>() << " done" << std::endl;
    }

    /// @brief Hands out one slot or all slots per allocation
    /// @details If you use this function otherwise, your program will be aborted
    /// @param no_slots 
    /// @return pointer to free slot  
    void *Allocate(size_t no_slots) {
        IsPoolCreatedCheck();
        return AllocateImpl(no_slots);
    }

    /// @brief Hands back one or all slots 
    /// @details If you use this function otherwise, your program will be aborted
    /// @param slot 
    /// @param no_slots 
    void Deallocate(void *slot, size_t no_slots) {
        IsPoolCreatedCheck();
        DeallocateImpl(slot, no_slots);
    }

    /// @brief  IsPoolCreated
    /// @return true if pool has been created, otherwise false 
    bool IsPoolCreated() {
        return is_pool_created_;
    }

    /// @brief GetSizeOfType
    /// @return size of type
    size_t GetSizeOfType() {
    IsPoolCreatedCheck();
        return sizeof(T);
    }

    /// @brief GetSizeOfPool
    /// @return number of slots in pool 
    size_t GetSizeOfPool() {
        IsPoolCreatedCheck();
        return no_slots_;
    }

private:
    PoolAllocator() :  no_slots_{0}, is_pool_created_{false}, pool_begin_{nullptr}, alloc_ptr_{nullptr}, slot_index_{0} { 
        std::cout << __FUNCTION__ << "<" << GetTypeName<T>() << ">" << std::endl;
    }
    ~PoolAllocator() {
        std::cout << __FUNCTION__ << "<" << GetTypeName<T>() << ">" << std::endl;
        assert(pool_begin_ == nullptr);
    }

    PoolAllocator(const PoolAllocator&)= delete;
    PoolAllocator& operator=(const PoolAllocator&)= delete;

  
    // A slots in a block contains the address of the next free slot
    struct Slot {
        Slot *next;
    };

    size_t no_slots_;
    bool is_pool_created_;
    Slot *pool_begin_;
    Slot *alloc_ptr_;
    size_t slot_index_;

    static_assert(sizeof(T) >= sizeof(Slot));

    void IsPoolCreatedCheck() {
        if(!IsPoolCreated()) {
            std::cout << "A memory pool for : " << GetTypeName<T>() << " does not exsit" << std::endl;
        }
    }

    Slot * CreateMemoryPoolImpl(size_t pool_size) {
        std::cout << __FUNCTION__ << std::endl;
        // The first slot of the pool
        Slot *pool_begin = reinterpret_cast<Slot *>(::operator new(pool_size*sizeof(T)));
        // Once the pool is allocated slots are ch
        Slot *slot = pool_begin;
        std::cout << "slot " << 1 << " @" << slot << std::endl;

        for (size_t i = 0; i < pool_size - 1; ++i) {
            slot->next =
                reinterpret_cast<Slot *>(reinterpret_cast<char *>(slot) + sizeof(T));
            slot = slot->next;
            std::cout << "slot " << i+2 << " @" << slot << std::endl;
        }

        slot->next = nullptr; // end of pool
        is_pool_created_ = true;
        std::cout << __FUNCTION__ << " Memory pool is created" << std::endl;
        return pool_begin;
    }

    void *AllocateImpl(size_t no_slots) {

        assert(no_slots == 1 || no_slots == no_slots_);

        if (alloc_ptr_ == nullptr) {
            std::cout << "No free slot in pool" << std::endl;;
            std::abort();
        }
                
        // The return value is the current position of
        // the allocation pointer:
        Slot *freeSlot = alloc_ptr_;
        std::cout << "freeSlot @" << static_cast<void*>(freeSlot) << std::endl;
        
        // Advance (bump) the allocation pointer to the next free slot.
        // When no slots left, the `alloc_ptr_` will be set to `nullptr`
        std::cout << __FUNCTION__ << " no_slots : " << no_slots << std::endl;
        std::cout << __FUNCTION__ << " slot_index_ + no_slots : " << slot_index_ + no_slots  << std::endl; 
        if(slot_index_ + no_slots-1 < no_slots_)
        {
            alloc_ptr_ = (alloc_ptr_+no_slots-1)->next;
            std::cout << __FUNCTION__ << " alloc_ptr_ : " << alloc_ptr_ << std::endl;
        } else {
            std::cout << "Not enought slots left in pool" << std::endl;
            std::abort();
        }
        
        slot_index_ +=no_slots;
        std::cout << __FUNCTION__ << " slot_index_ : " <<  slot_index_ << std::endl;

        return freeSlot;
    }

    void DeallocateImpl(void *slot, size_t no_slots) {
        std::cout << __FUNCTION__ << " for type " << GetTypeName<T>() << std::endl;
        std::cout << __FUNCTION__ << " no_slots : " << no_slots << std::endl; 
        assert(slot != nullptr);

        if(pool_begin_ != nullptr) {

            assert(no_slots == 1 || no_slots == no_slots_);
        
            std::cout << __FUNCTION__ << " alloc_ptr_ : " << alloc_ptr_ << std::endl;
            Slot * tmp_slot = reinterpret_cast<Slot *>(slot);
            if(no_slots-1 >0) {
                std::cout << tmp_slot << std::endl;
                for (size_t i = 0; i < no_slots-1; i++)
                {
                tmp_slot->next = reinterpret_cast<Slot *>(reinterpret_cast<char *>(tmp_slot) + sizeof(T));
                tmp_slot = tmp_slot->next;
                std::cout << tmp_slot << std::endl;;
                }
            } 
            
            reinterpret_cast<Slot *>(tmp_slot)->next = alloc_ptr_;

            alloc_ptr_ = reinterpret_cast<Slot *>(slot);
            std::cout << __FUNCTION__ << " alloc_ptr_ : " << alloc_ptr_ << std::endl;
            std::cout << __FUNCTION__ << " for type " << GetTypeName<T>() << "done" << std::endl;
        } else {
            std::cout << __FUNCTION__ << " poll has been deallocated already" << std::endl;
        }
    }
        
};


#if 0
struct Object {
 
  // Object data, 16 bytes:
 
  uint64_t data[2];
 
  // Declare out custom allocator for
  // the `Object` structure:
 
  static void *operator new(size_t size) {
    return PoolAllocator<Object>::Get().Allocate(size);
  }
 
  static void operator delete(void *ptr, size_t size) {
    return PoolAllocator<Object>::Get().Deallocate(ptr, size);
  }
};

#endif


template <class T>
class Allocator
{
    static_assert(!std::is_volatile<T>::value, "Allocator does not support volatile types");
    public:
        typedef size_t    size_type;
        typedef ptrdiff_t difference_type;
        typedef T*        pointer;                         
        typedef const T*  const_pointer;                   
        typedef T&        reference;
        typedef const T&  const_reference;
        typedef T         value_type;


        typedef std::true_type propagate_on_container_move_assignment;
        typedef std::true_type is_always_equal;

        Allocator() noexcept = default;                      
        Allocator(const Allocator&) noexcept {};      
        template <class U>
        constexpr Allocator(const Allocator<U>&) noexcept {} 
        //~Allocator();                                        

    
        T* allocate(size_t no_slots) {    
            if(no_slots > std::allocator_traits<Allocator>::max_size(*this)) {
                std::cout << "request not possible" << std::endl;
                std::abort();
            }
            return static_cast<T*>(PoolAllocator<T>::Get().Allocate(no_slots));
        }

        void deallocate(T* p, size_t no_slots) noexcept {
            PoolAllocator<T>::Get().Deallocate(p,no_slots);
        }

        template <class U>
        struct  rebind {
            typedef Allocator<U> other;
        };

        pointer address(reference x) const noexcept {            
            return std::addressof(x);
        }

        const_pointer address(const_reference x) const noexcept { 
            return std::addressof(x);
        }

        
        T* allocate(size_t no_slots, const void* ){         
            return allocate(no_slots);
        }

        size_type max_size() const noexcept {              
            return size_type(~0) / sizeof(T);
        }
        template<class U, class... Args>
        void construct(U* p, Args&&... args){         
            ::new ((void*)p) U(std::forward<Args>(args)...);
        }

        void destroy(pointer p) {                           
             p->~T();
        }
};

template <class T, class U>
bool operator==(const Allocator<T>&, const Allocator<U>&) noexcept { 
    return true;
}

template <class T, class U>
bool operator!=(const Allocator<T>&, const Allocator<U>&) noexcept {
    return false;
}

// how to make Unique Pools?

struct ClassName
{
    ClassName () { 
        //PoolAllocator<ClassNameAllocatorInfoContainer>::Get().CreateMemoryPool(10); // how make pools unique for a type? 
        //v.reserve(pool_size);
    }
    ~ClassName () {
         //PoolAllocator<ClassNameAllocatorInfoContainer>::Get().ReleaseMemoryPool();
    } 
    struct ClassNameAllocatorInfoContainer
    {
        using type_info = uint64_t;
    };
    static_assert(sizeof(ClassNameAllocatorInfoContainer) == 1);
    //std::vector<ClassNameAllocatorInfoContainer, Allocator<ClassNameAllocatorInfoContainer>> v;
    size_t pool_size = 10; 
};
    

int main() {
    using vec = std::vector<double, Allocator<double>>;  
    size_t no_slots_double = 10;
    PoolAllocator<double>::Get().CreateMemoryPool(no_slots_double); // how make pools unique for a type? 
    // Making the class Part of the type ??

    vec v;
    //v.get_allocator().reserve(10);
    v.reserve(no_slots_double); // <- required due to vectors allocation strategie
    // the alternative would be to create a bigger pool then the vector would hold 
    // additionally the pool allocator would require to search for a big enough free chunck of successive 
    // free slots. 
    for(int i = 0; i<10; i++) {
      v.push_back(i);
    }

    for(int i = 0; i < 10; i++) {
      assert(v[i] == static_cast<double>(i));
    }

    v.clear();

    for(int i = 0; i<10; i++) {
      v.push_back(i);
    }

    for(int i = 0; i < 10; i++) {
      assert(v[i] == static_cast<double>(i));
    }


    v.clear();

    for(int i = 5; i<10; i++) {
      v.push_back(i);
    }
  
    v.emplace(v.begin(), 4);
    v.emplace(v.begin(), 3);
    v.emplace(v.begin(), 2);
    v.emplace(v.begin(), 1);
    v.emplace(v.begin(), 0);

    for(int i = 0; i<10; ++i) {
        assert(i == static_cast<int>(v[i]));
    }
    PoolAllocator<double>::Get().ReleaseMemoryPool();


    using Key = uint64_t;
    using T = uint64_t;
    using Compare = std::less<Key>;
    using map = std::map<Key, T, Compare, Allocator<std::pair<const Key, T>>>;

    using GCCInternalTypeForAMapElement = std::_Rb_tree_node<std::pair<const Key, T>>;

    const size_t no_slots_map = 5; 
    PoolAllocator<std::_Rb_tree_node<std::pair<const Key, T>>>::Get().CreateMemoryPool(no_slots_map);
    
    map m;

    static_assert(std::is_same_v<map::node_type::key_type, Key>);
    static_assert(std::is_same_v<map::node_type::mapped_type, T>);
    for (T i = 1; i <= no_slots_map; ++i)
    {
      m[i] = i*i;
    }

    for (const auto& n : m) {
        assert(n.first*n.first == n.second);
    }

    PoolAllocator<GCCInternalTypeForAMapElement>::Get().ReleaseMemoryPool();

    std::cout << "End of main" << std::endl;
}
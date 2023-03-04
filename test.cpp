
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
#include <bits/stl_tree.h>

#define DEBUG_BUMPALO

/// @brief GetTypeName
/// @tparam T 
/// @return returns the demangled name of T
template <class T>
std::string GetTypeName() {
    auto m_name = typeid(T).name();
    size_t buff_size = sizeof(m_name);
    auto buff = reinterpret_cast<char*>(::operator new(buff_size));
    int stat = 0;
    if(buff) {
        buff = abi::__cxa_demangle(m_name, buff, &buff_size, &stat);
    } else {
        std::abort();
    }
    std::string ret(buff);
    ::operator delete(buff);
    return ret;
}

class Allocator {

    public: 
        static Allocator & Get(){
            static Allocator instance;
            return instance;
        }

        void AllocAllow() {
            mem_freez_= false;
        }

        void AllocProhibit() {
            mem_freez_ = true;
        }

        bool IsAllocAllowed() {
            return mem_freez_;
        }

    private:
        Allocator() : mem_freez_{false} {}
        ~Allocator() {};

        bool mem_freez_;

};

/// @brief BumpAlo 
///         
/// @details
///  
///        pool : [slot][slot][slot][slot][slot][slot]
///
///        slot : [pointer to next free slot] or [T] 
///               Note: the last slot contains nullptr to mark the end of the pool.
///  
///        This allocator can only hand out one slot or all slots per allocation.
///        Only one slot can be handed back to the pool per deallocation.
///                
///        Rationale: This allocator is to be used in conjunction with 
///                   std::map. This containers serve the usecase to work with a 
////                  per element approach (see modifiers interface of std::map). 
///                   If this allocation strategie fits into your
///                   objects allocation demangs you are free to use it.  
/// 
///                
///        Dynamic memory, is a global resource. A object requests heap memory on initialisation 
///        and releases it on destruction. To keep the global character a thread safe singelton 
///        pattern is followed, allowing lazy initialization (Mayer's Singelton).
/// 
/// @attention 
/// 
///        Only one BumpAlo<T> instance will exsit in your programm and once 
///        createatd it will exsit during the entire programms duration. 
///
/// @tparam T 
template <typename T>
class BumpAlo {

    public:
    /// @brief Getter to the instance of BumpAlo<T>
    /// @attention never assign the returned instance to a variable. 
    /// @return use BumpAlo<T>::Get().Function() instead
    static BumpAlo & Get() {
        static BumpAlo instance;
        return instance;
    }

    /// @brief Adds a new block of memory for the pool of T
    /// @details   The global new operator is used to request memory for the pool.
    ///            no_slots slots are added to the pool for T.
    /// @param no_slots 
    void AddMemory(size_t no_slots = 1) {

            Slot * block_begin = AddMemoryImpl(no_slots);
            if(no_blocks_== 0) {
                alloc_ptr_ = block_begin;
            }

            if(alloc_ptr_ == nullptr) {
                alloc_ptr_ = block_begin;
            }

            ++no_blocks_;
            no_slots_ += no_slots;
            block_size_ = no_slots;
            free_ptr_.push_back(block_begin);
    }

      
    /// @brief Hands out one slot per allocation
    /// @details If this function is used otherwise, your program will be aborted
    /// @param no_slots 
    /// @return pointer to free slot  
    T *Allocate(size_t no_slots = 1) {

        assert(no_slots == 1);
        if (alloc_ptr_ == nullptr) {
            block_size_ = 1;
            alloc_ptr_ = AddMemoryImpl(block_size_);
            ++no_blocks_;
            no_slots_ += no_slots;
            free_ptr_.push_back(alloc_ptr_);
        } 
        Slot *free_slot = alloc_ptr_;
        alloc_ptr_ = alloc_ptr_->next;

        return reinterpret_cast<T*>(free_slot);
    }

    /// @brief Hands back one slot
    /// @details If this function is used otherwise, your program will be aborted
    /// @param slot 
    /// @param no_slots 
    void Deallocate(void *slot, size_t no_slots = 1) {
        assert(no_slots == 1);
        reinterpret_cast<Slot *>(slot)->next = alloc_ptr_;
        alloc_ptr_ = reinterpret_cast<Slot *>(slot);
    }


    /// @brief GetSizeOfType
    /// @return size of type
    size_t GetSizeOfType() {
        return sizeof(T);
    }

    /// @brief GetSizeOfPool
    /// @return number of slots in pool 
    size_t GetSizeOfPool() {

        return no_slots_;
    }

    /// @brief GetNoOfBlocks
    /// @return number of blocks added to the pool for type T
    size_t GetNoOfBlocks() {
        return no_blocks_;
    }

private:
    BumpAlo() :  no_slots_{0},  no_blocks_{0}, block_size_{1}, alloc_ptr_{nullptr}, block_end_{nullptr}, type_name_{GetTypeName<T>()} { 
        #ifdef DEBUG_BUMPALO
            std::cout << __FUNCTION__ << "<" << type_name_<< ">" << std::endl;
        #endif
    }
    ~BumpAlo() {
        #ifdef DEBUG_BUMPALO
            std::cout << __FUNCTION__ << "<" << type_name_<< ">" << std::endl;
        #endif
        for(auto ptr : free_ptr_) {
            ::operator delete(ptr);
        } 
    }

    BumpAlo(const BumpAlo&)= delete;
    BumpAlo& operator=(const BumpAlo&)= delete;

  
    // A slots in a block contains the address of the next free slot
    struct Slot {
        Slot *next;
    };

    size_t no_slots_;
    size_t no_blocks_;
    size_t block_size_;
    Slot *alloc_ptr_;
    Slot *block_end_;
    const std::string type_name_;
    std::vector<void*> free_ptr_;

    static_assert(sizeof(T) >= sizeof(Slot));


    Slot * AddMemoryImpl(size_t block_size) {
        assert(block_size_>=1);
        // The first slot of the block
        Slot *block_begin = reinterpret_cast<Slot *>(::operator new(block_size*sizeof(T)));
        Slot *slot = block_begin;

        for (size_t i = 0; i < block_size - 1; ++i) {
            slot->next =  reinterpret_cast<Slot *>(reinterpret_cast<char *>(slot) + sizeof(T));
            slot = slot->next;
           
        }

        slot->next = nullptr;

        if(alloc_ptr_ != nullptr) {
            block_end_->next =  reinterpret_cast<Slot *>(block_begin);
        }

        block_end_ = slot;

        return block_begin;
    }       
};

template <class T>
class PAlo
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

        PAlo() noexcept = default;                      
        PAlo(const PAlo&) noexcept {};      
        template <class U>
        constexpr PAlo(const PAlo<U>&) noexcept {} 
        //~Allocator();                                        

        T* allocate(size_t no_slots) {    
            if(no_slots > std::allocator_traits<PAlo>::max_size(*this)) {
                //std::cout << "request not possible" << std::endl;
                std::abort();
            }
            return static_cast<T*>(BumpAlo<T>::Get().Allocate(no_slots));
        }

        void deallocate(T* p, size_t no_slots) noexcept {
            BumpAlo<T>::Get().Deallocate(p,no_slots);
        }

        template <class U>
        struct  rebind {
            typedef PAlo<U> other;
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
bool operator==(const PAlo<T>&, const PAlo<U>&) noexcept { 
    return true;
}

template <class T, class U>
bool operator!=(const PAlo<T>&, const PAlo<U>&) noexcept {
    return false;
}


template <class T>
class ShmemAlo
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

        ShmemAlo() noexcept = default;                      
        ShmemAlo(const ShmemAlo&) noexcept {};      
        template <class U>
        constexpr ShmemAlo(const ShmemAlo<U>&) noexcept {}                                     

    
        T* allocate(size_t no_slots) {    
            if(no_slots > std::allocator_traits<ShmemAlo>::max_size(*this)) {
                std::abort();
            }
            return static_cast<T*>(BumpAlo<T>::Get().Allocate(no_slots)); // <- replace with shmem 
        }

        void deallocate(T* p, size_t no_slots) noexcept {
            BumpAlo<T>::Get().Deallocate(p,no_slots); // <- replace with shmem 
        }

        template <class U>
        struct  rebind {
            typedef ShmemAlo<U> other;
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
bool operator==(const ShmemAlo<T>&, const ShmemAlo<U>&) noexcept { 
    return true;
}

template <class T, class U>
bool operator!=(const ShmemAlo<T>&, const ShmemAlo<U>&) noexcept {
    return false;
}


int main() {

    {
        using Key = uint64_t;
        using T = uint64_t;
        using Compare = std::less<Key>;
        using Type = std::pair<const Key, T>;

        using GCCInternalTypeForAMapElement = std::_Rb_tree_node<Type>;

        using Map = std::map<Key, T, Compare, PAlo<Type>>;

        const size_t no_slots_map = 5; 
        BumpAlo<GCCInternalTypeForAMapElement>::Get().AddMemory(no_slots_map);
        
        Map m;

        for (T i = 1; i <= no_slots_map; ++i)
        {
            m[i] = i*i;
        }

        for (const auto& n : m) {
            assert(n.first*n.first == n.second);
        }


        Map::iterator begin2 = Map::iterator(m.begin());
        Map::iterator end2 = Map::iterator(m.end());

        Map m2(begin2, end2);

        for (const auto& n : m2) {
            assert(n.first*n.first == n.second);
        }


        using Map2= std::map<Key, T, Compare, ShmemAlo<Type>>;

        BumpAlo<GCCInternalTypeForAMapElement>::Get().AddMemory(no_slots_map);
        

        Map2::iterator begin3;
        Map2::iterator end3;
        
        Map2 m3(begin2, end2);

        for (const auto& n : m3) {
            assert(n.first*n.first == n.second);
        }

        begin3 = m3.begin();
        end3 = m3.end();
        


        for(Map2::iterator it = begin3; it != end3; ++it) {
            auto first = it->first;
            auto second = it->second;
            assert(first*first == second);
        } 
        

        
        Map2::iterator begin4;
        Map2::iterator end4;
        
        Map2 m4;

        for (T i = 1; i <= no_slots_map; ++i)
        {

            m4[i+5] = (i+5)*(i+5);
        }


        for (const auto& n : m4) {
            assert(n.first*n.first == n.second);
        }

        begin4 = m4.begin();
        end4 = m4.end();
        


        for(Map2::iterator it = begin4; it != end4; ++it) {
            auto first = it->first;
            auto second = it->second;
            assert(first*first == second);
        } 
    }  

    {
        // validating bumpalo
        bool typeCheck = std::string("bool") == GetTypeName<bool>();
        assert(typeCheck);

        struct TestType1{
            TestType1(uint64_t x) : x_{x} {}
            uint64_t x_;
        };

        using ba1 = BumpAlo<TestType1>;

        ba1::Get().AddMemory();
        ba1::Get().AddMemory();
        ba1::Get().AddMemory();

        assert(ba1::Get().GetSizeOfPool() == 3);
        assert(ba1::Get().GetNoOfBlocks() == 3);

        ba1::Get().Allocate();
        ba1::Get().Allocate();
        ba1::Get().Allocate();

        assert(ba1::Get().GetSizeOfPool() == 3);
        assert(ba1::Get().GetNoOfBlocks() == 3);

        ba1::Get().Allocate();

        assert(ba1::Get().GetSizeOfPool() == 4);
        assert(ba1::Get().GetNoOfBlocks() == 4);

        ba1::Get().AddMemory(2);
        assert(ba1::Get().GetSizeOfPool() == 6);
        assert(ba1::Get().GetNoOfBlocks() == 5);

        auto p1 = ba1::Get().Allocate();
        assert(ba1::Get().GetSizeOfPool() == 6);
        assert(ba1::Get().GetNoOfBlocks() == 5);

        auto p2 = ba1::Get().Allocate();
        assert(ba1::Get().GetSizeOfPool() == 6);
        assert(ba1::Get().GetNoOfBlocks() == 5);

        ::new ((void*)p1) TestType1(1);
        ::new ((void*)p2) TestType1(32);

        assert(p1->x_ == 1);
        assert(p2->x_ == 32);


        ba1::Get().Allocate();
        assert(ba1::Get().GetSizeOfPool() == 7);
        assert(ba1::Get().GetNoOfBlocks() == 6);
        
        assert(p1->x_ == 1);
        assert(reinterpret_cast<TestType1*>(( reinterpret_cast<char *>(p1) + sizeof(TestType1) ))->x_ == 32);
        ba1::Get().Deallocate(p1);
        assert(p1->x_ != 1);
        assert(reinterpret_cast<TestType1*>(( reinterpret_cast<char *>(p1) + sizeof(TestType1) ))->x_ == 32);


        auto p3 = ba1::Get().Allocate();
        ::new ((void*)p3) TestType1(1);
        assert(reinterpret_cast<TestType1*>(( reinterpret_cast<char *>(p3) + sizeof(TestType1) ))->x_ == 32);
        assert(p1 == p3);

        auto p4 = ba1::Get().Allocate();
        assert(reinterpret_cast<TestType1*>(( reinterpret_cast<char *>(p4) - sizeof(TestType1) ))->x_ != 1);
    }
}
  
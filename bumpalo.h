#ifndef BUMPALO_H
#define BUMPALO_H

#include <iostream>
#include <vector>
#include "typename.h"

/// @brief BumpAlo 
///         
/// @details
///  
///        pool : [slot][slot][slot][slot][slot][slot]
///
///        slot : [pointer to next free slot] or [T] 
///               Note: the last slot contains nullptr to mark the end of the pool.
///  
///        BumpAlo can only hand out one slot per allocation request.
///        Only one slot can be handed back to the pool per deallocation.
///        The AddMemory function can be used pre-allocate memory. If the pool is 
///        exhausted Allocate will call AddMemory.
///                
///        Rationale: This allocator is to be used in conjunction with 
///                   std::map. This container serve the usecase to work with a 
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
///        createatd it will exsit during the entire programms duration. Many 
///        objects can use BumpAlo<T>'s memory pool.
///
/// @tparam T 
template <class T>
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
    /// @details The global new operator is used to request memory for the pool.
    ///           no_slots slots are added to the pool for T.
    /// @param no_slots 
    void AddMemory(size_t no_slots = 1) {

            Slot * block_begin = AddMemoryImpl(no_slots);

            // first block of the pool 
            // allocation ptr is set to block begin
            if(no_blocks_== 0) {
                alloc_ptr_ = block_begin;
            }

            // pool is exhauseted new memory is added to the 
            // allocation ptr is set to block begin  
            if(alloc_ptr_ == nullptr) {
                alloc_ptr_ = block_begin;
            }

            // every call to AddMemory add a new block of size no_slots
            ++no_blocks_;
            no_slots_ += no_slots;
            block_size_ = no_slots;

            // storing block_begin
            // to release the memory back to the OS
            // the the end of the programm
            ptr_to_free_.push_back(static_cast<void*>(block_begin));
    }

      
    /// @brief Hands out one slot per allocation
    /// @details If this function is used otherwise, the program will be aborted
    /// @param no_slots 
    /// @return pointer to free slot  
    T *Allocate(size_t no_slots = 1) {
        if(no_slots != 1) {
            std::cerr << __FUNCTION__ << " can hand out only one slot per allocation request\n";
            std::abort();
        }
        // no block as been crated yet 
        if (alloc_ptr_ == nullptr) {
            block_size_ = 1;
            alloc_ptr_ = AddMemoryImpl(block_size_);
            ++no_blocks_;
            no_slots_ += no_slots;
            ptr_to_free_.push_back(static_cast<void*>(alloc_ptr_));
        } 
        Slot *free_slot = alloc_ptr_;
        alloc_ptr_ = alloc_ptr_->next;
                
#ifdef DEBUG_BUMPALO
            std::cout << __FUNCTION__ << "<" << type_name_<< "> \n      free_slot @" << free_slot << std::endl;
#endif

        return reinterpret_cast<T*>(free_slot);
    }

    /// @brief Hands back one slot
    /// @details If this function is used otherwise, the program will be aborted
    /// @param slot 
    /// @param no_slots 
    void Deallocate(void *slot, size_t no_slots = 1) {
        if(no_slots != 1) {
            std::cerr << __FUNCTION__ << " can hand back only one slot per deallocation\n";
            std::abort();
        }
        new (slot) Slot(); // deleting slot's content 
        reinterpret_cast<Slot *>(slot)->next = alloc_ptr_;
        alloc_ptr_ = reinterpret_cast<Slot *>(slot);

#ifdef DEBUG_BUMPALO
            std::cout << __FUNCTION__ << "<" << type_name_<< "> \n      deleted @" << slot << std::endl;
#endif
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
#ifdef DEBUG_BUMPALO
    BumpAlo() :  no_slots_{0},  no_blocks_{0}, block_size_{1}, alloc_ptr_{nullptr}, block_end_{nullptr}, type_name_{GetTypeName<T>()} { 
           std::cout << __FUNCTION__ << "<" << type_name_<< ">" << std::endl;
    }
#else
     BumpAlo() :  no_slots_{0},  no_blocks_{0}, block_size_{1}, alloc_ptr_{nullptr}, block_end_{nullptr} {}
#endif

    ~BumpAlo() {
        for(auto ptr : ptr_to_free_) {
            ::operator delete(ptr);
        }

#ifdef DEBUG_BUMPALO
        std::cout << __FUNCTION__ << "<" << type_name_<< ">" << std::endl;
#endif 
    }

    BumpAlo(const BumpAlo&)= delete;
    BumpAlo& operator=(const BumpAlo&)= delete;

  
    // A slots in a block contains the address of the next free slot

    struct alignas(alignof(T)) Slot {
        Slot *next;
    };

    size_t no_slots_;
    size_t no_blocks_;
    size_t block_size_;
    Slot *alloc_ptr_;
    Slot *block_end_;
#ifdef DEBUG_BUMPALO
    const std::string type_name_;
#endif
    std::vector<void*> ptr_to_free_;

    static_assert(sizeof(T) >= 8, "Smaller types are not supported");


    Slot * AddMemoryImpl(size_t block_size) {
        if(block_size <= 0) {
            std::cerr << __FUNCTION__ << " block_size : " << block_size << " is not possible\n";
            std::abort();
        }

        // request memeory from OS
        Slot *block_begin = reinterpret_cast<Slot *>(::operator new(block_size*sizeof(T)));
        // start of block 
        Slot *slot = block_begin;

        // crate slots
        for (size_t i = 0; i < block_size - 1; ++i) {
            slot->next =  reinterpret_cast<Slot *>(reinterpret_cast<char *>(slot) + sizeof(T));
            slot = slot->next;
           
        }

        // last slot of pool
        slot->next = nullptr;

        // connect blocks, if pool is not exhausted already 
        if(alloc_ptr_ != nullptr) {
            block_end_->next =  reinterpret_cast<Slot *>(block_begin);
        }   


        block_end_ = slot;

        return block_begin;
    }       
};

#endif // BUMPALO_H
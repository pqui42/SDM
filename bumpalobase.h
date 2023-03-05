#ifndef BUMPALOBASE_H
#define BUMPALOBASE_H

#include <iostream>
#include <vector>
#include "typename.h"


template <class T>
class BumpAloBase {

public:

#ifdef DEBUG_BUMPALOBASE
    BumpAloBase() :  no_slots_{0},  no_blocks_{0}, block_size_{1}, alloc_ptr_{nullptr}, block_end_{nullptr}, type_name_{GetTypeName<T>()} { 
           std::cout << __FUNCTION__ << "<" << type_name_<< ">" << std::endl;
    }
#else
     BumpAloBase() :  no_slots_{0},  no_blocks_{0}, block_size_{1}, alloc_ptr_{nullptr}, block_end_{nullptr} {}
#endif

    ~BumpAloBase() {
        for(auto ptr : ptr_to_free_) {
            ::operator delete(ptr);
        }

#ifdef DEBUG_BUMPALOBASE
        std::cout << __FUNCTION__ << "<" << type_name_<< ">" << std::endl;
#endif 
    }

    BumpAloBase(const BumpAloBase&)= delete;
    BumpAloBase& operator=(const BumpAloBase&)= delete;

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

            // every call to AddMemory adds a new block of size no_slots
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
        if(no_blocks_ == 0) {
            std::cerr << __FUNCTION__ << " pool has not been created yet\n";
            std::abort();
        }

        if (alloc_ptr_ == nullptr) {
            std::cerr << __FUNCTION__ << " no free slots in pool.\n";
            std::abort();
        } 
        Slot *free_slot = alloc_ptr_;
        alloc_ptr_ = alloc_ptr_->next;
                
#ifdef DEBUG_BUMPALOBASE
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

#ifdef DEBUG_BUMPALOBASE
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
    
    bool IsEndOfBlock() {
        if(no_blocks_ == 0) {
            std::cerr << __FUNCTION__ << " pool has not been created yet\n";
            std::abort();
        }
        return alloc_ptr_ == nullptr;
    }

private:  
    // A slots in a block contains the address of the next free slot

    struct alignas(alignof(T)) Slot {
        Slot *next;
    };

    size_t no_slots_;
    size_t no_blocks_;
    size_t block_size_;
    Slot *alloc_ptr_;
    Slot *block_end_;
#ifdef DEBUG_BUMPALOBASE
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

#endif // BUMPALOBASE_H
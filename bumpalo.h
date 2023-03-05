#ifndef BUMPALO_H
#define BUMPALO_H

#include "bumpalobase.h"

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
        base_.AddMemory(no_slots);
    }

      
    /// @brief Hands out one slot per allocation
    /// @details If this function is used otherwise, the program will be aborted
    /// @param no_slots 
    /// @return pointer to free slot  
    T *Allocate(size_t no_slots = 1) {
        if(base_.GetNoOfBlocks() == 0) {
            base_.AddMemory(1);
        } else if(base_.IsEndOfBlock()) {
            base_.AddMemory(1);
        }
        return base_.Allocate(no_slots);
    }

    /// @brief Hands back one slot
    /// @details If this function is used otherwise, the program will be aborted
    /// @param slot 
    /// @param no_slots 
    void Deallocate(void *slot, size_t no_slots = 1) {
        base_.Deallocate(slot, no_slots);
    }


    /// @brief GetSizeOfType
    /// @return size of type
    size_t GetSizeOfType() {
        return base_.GetSizeOfType();
    }

    /// @brief GetSizeOfPool
    /// @return number of slots in pool 
    size_t GetSizeOfPool() {
        return base_.GetSizeOfPool();
    }

    /// @brief GetNoOfBlocks
    /// @return number of blocks added to the pool for type T
    size_t GetNoOfBlocks() {
        return base_.GetNoOfBlocks();
    }

private:

    BumpAlo() { 
#ifdef DEBUG_BUMPALO
           std::cout << __FUNCTION__ << std::endl;
#endif
    }

    ~BumpAlo() {

#ifdef DEBUG_BUMPALO
        std::cout << __FUNCTION__  << std::endl;
#endif 
    }

    BumpAlo(const BumpAlo&)= delete;
    BumpAlo& operator=(const BumpAlo&)= delete;

    BumpAloBase<T> base_;
  
};



#endif // BUMPALO_H
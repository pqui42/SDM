
#include <cassert>
#include <typeindex>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <memory>
#include <iostream>


template <typename T>
class Test {
public:
  void CreateMemoryPool(size_t pool_size){
    if(is_pool_created_) {
      std::cerr << "Pool already created\n";
      std::abort();
    } else {
      type_size_ = sizeof(T);
      pool_size_ = pool_size;
      // crate pool 
      is_pool_created_ = true;
    }
  }

  static Test & Get() {
    static Test instance;
    return instance;
  }

  size_t GetSizeOfType() {
   PoolIsCreated();
    return type_size_;
  }

  size_t GetSizeOfPool() {
    PoolIsCreated();
    return pool_size_;
  }
private:
  void PoolIsCreated() {
      if(!is_pool_created_) {
      std::cerr << "Pool not already created\n";
      std::abort();
    }
  }
  Test() = default;
  ~Test() = default;
  Test(const Test&)= delete;
  Test& operator=(const Test&)= delete;
  size_t pool_size_;
  size_t type_size_;
  bool is_pool_created_ = false;
};

int main() {
  Test<int>::Get().CreateMemoryPool(100);
  auto typeSize = Test<int>::Get().GetSizeOfType();
  auto poolSize = Test<int>::Get().GetSizeOfPool();

  assert(typeSize == sizeof(int));
  assert(poolSize == 100);
}

#if 0
// A block constists no_solts_per_block_
struct Slot {
  // A slots in a block contains the address of the next free slot
  Slot *next;
};

constexpr size_t min_size_of_slot = sizeof(Slot);

static_assert(sizeof(min_size_of_slot) == 8); // a 64 bit machine is assumend


// The PoolAllocator class
// 
// Features:
// 
//   - Parametrized by number of splot per block
//   - Keeps track of the allocation pointer
//   - Bump-allocates slots
template<typename T>
class PoolAllocator {
 public:
    PoolAllocator(size_t no_solts_per_block)
    : no_solts_per_block_(no_solts_per_block)
    {

    }

  ~PoolAllocator() {
    free(pool_);
  }
 
  // retruns pointer to next free slot or
  // creates a new pool
  void *Allocate(size_t size);
  // 
  void Deallocate(void *pointer, size_t size);

 private:

  // number of slots per larger block.
  size_t no_solts_per_block_;
 
  // allocation pointer
  // points to next free slot, else nullptr 
  Slot *alloc_ = nullptr;
  
  // pointer to free memory pool.
  void * pool_= nullptr;

  bool allocate_block = true;

  // Allocates a no_solts_per_block_*slot_size as block
  // of memory and creates a pool of memory 
  Slot *AllocateBlock(size_t slot_size);
};

template<typename T>
Slot *PoolAllocator<T>::AllocateBlock(size_t slot_size) {

  if(allocate_block) {
    allocate_block = false;
  } else {
    std::abort();
  }

  if(slot_size < min_size_of_slot) {
    slot_size = min_size_of_slot;
  }
 
  size_t block_size = no_solts_per_block_ * slot_size;
 
  // The first slot of the new block.
  Slot *block_begin = reinterpret_cast<Slot *>(malloc(block_size));
  pool_ = static_cast<void *>(block_begin);
 
  // Once the block is allocated, we need to chain all
  // the slots in this block:
 
  Slot *slot = block_begin;

  for (size_t i = 0; i < no_solts_per_block_ - 1; ++i) {
    slot->next =
        reinterpret_cast<Slot *>(reinterpret_cast<char *>(slot) + slot_size);
    slot = slot->next;
  }
 
  slot->next = nullptr; // end of block

  return block_begin;
}

template<typename T>
void *PoolAllocator<T>::Allocate(size_t size) {

  if(size < min_size_of_slot) {
    size = min_size_of_slot;
  }

  // No slots left in the current block, or 
  // a block of memory has not been created yet.
  if (alloc_ == nullptr) {
    alloc_ = AllocateBlock(size);
  }
 
  // The return value is the current position of
  // the allocation pointer:
  Slot *freeSlot = alloc_;
 
  // Advance (bump) the allocation pointer to the next slot.
  //
  // When no slots left, the `alloc_` will be set to `nullptr`, and
  // this will cause allocation of a new block on the next request:
 
  alloc_ = alloc_->next;

  return freeSlot;
}
template<typename T>
void PoolAllocator<T>::Deallocate(void *slot, size_t size) {

  if(size < min_size_of_slot) {
    size = min_size_of_slot;
  }
 
  // The freed slots's next pointer points to the
  // current allocation pointer:
 
  reinterpret_cast<Slot *>(slot)->next = alloc_;
 
  // And the allocation pointer is now set
  // to the returned (free) chunk:
 
  alloc_ = reinterpret_cast<Slot *>(slot);
}

template<typename T>
class Allocator {
public : 
    //    typedefs
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

public : 
    //    convert an allocator<T> to allocator<U>
    template<typename U>
    struct rebind {
        typedef Allocator<U> other;
    };

public : 
    inline explicit Allocator() {}
    inline ~Allocator() {}
    inline explicit Allocator(Allocator const&) {}
    template<typename U>
    inline explicit Allocator(Allocator<U> const&) {}

    //    address
    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_reference r) { return &r; }

    //    memory allocation
    inline pointer allocate(size_type cnt, 
       typename std::allocator<void>::const_pointer = 0) { 
      return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); 
    }
    inline void deallocate(pointer p, size_type) { 
        ::operator delete(p); 
    }

    //    size
    inline size_type max_size() const { 
        return std::numeric_limits<size_type>::max() / sizeof(T);
 }

    //    construction/destruction
    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }

    inline bool operator==(Allocator const&) { return true; }
    inline bool operator!=(Allocator const& a) { return !operator==(a); }
};    //    end of class Allocator 


int main() {

  static PoolAllocator<int>  p(100);

  auto p = p.Allocate(sizeof(int));

}

#endif
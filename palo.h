#ifndef PALO_H
#define PALO_H

#include "bumpalo.h"

template <class T>
class PAlo {   
    static_assert(!std::is_volatile<T>::value, "PAlo does not support volatile types");
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
                std::cerr << __FUNCTION__ << " request not possible" << std::endl;
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
            ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
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


#endif // PALO_H
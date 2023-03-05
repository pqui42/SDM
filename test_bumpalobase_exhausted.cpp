#include <cassert>
#include "typename.h"
#include "bumpalobase.h"

int main() {

        struct TestType1{
            TestType1(uint64_t x) : x_{x} {}
            uint64_t x_;
        };

        BumpAloBase<TestType1> ba;

        uint64_t no_slots = 6;
        ba.AddMemory(no_slots);
        
        for(uint64_t i = 0; i<no_slots+1; ++i) {
            auto p = ba.Allocate();
            new (static_cast<void*>(p)) TestType1(i);
        }
}
  
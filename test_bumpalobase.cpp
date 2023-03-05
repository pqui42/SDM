#include <cassert>
#include "typename.h"
#include "bumpalobase.h"

int main() {

        struct TestType1{
            TestType1(uint64_t x) : x_{x} {}
            uint64_t x_;
        };

        BumpAloBase<TestType1> ba;

        assert(ba.GetNoOfBlocks() == 0);
        assert(ba.GetSizeOfPool() == 0);
        assert(ba.GetSizeOfType() == sizeof(TestType1));

        uint64_t no_slots = 6;
        ba.AddMemory(no_slots);

        assert(ba.GetNoOfBlocks() == 1);
        assert(ba.GetSizeOfPool() == no_slots);
        
        for(uint64_t i = 0; i<no_slots; ++i) {
            auto p = ba.Allocate();
            new (static_cast<void*>(p)) TestType1(i);
        }

        if(ba.IsEndOfBlock()){
            ba.AddMemory(no_slots);
        }
        
        assert(ba.GetNoOfBlocks() == 2);
        assert(ba.GetSizeOfPool() == no_slots*2);

        for(uint64_t i = 0; i<no_slots; ++i) {
            auto p = ba.Allocate();
            new (static_cast<void*>(p)) TestType1(i);
        }

        assert(ba.IsEndOfBlock());
}
  
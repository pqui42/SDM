#include <cassert>
#include "typename.h"
#include "palo.h"

int main() {

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
        ::new ((void*)p4) TestType1(123);
}
  
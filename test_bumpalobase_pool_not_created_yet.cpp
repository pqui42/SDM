#include <cassert>
#include "typename.h"
#include "bumpalobase.h"

int main() {

        struct TestType1{
            TestType1(uint64_t x) : x_{x} {}
            uint64_t x_;
        };

        BumpAloBase<TestType1> ba;  
        ba.Allocate();
}
  
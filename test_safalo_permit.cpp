

#include "safalo.h"
#include <cassert>

int main() {
    auto p = ::operator new(1);
    ::operator delete(p);
}
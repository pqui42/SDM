

#include "safalo.h"
#include <cassert>

int main() {
    SafAlo::Get().AloProhibit();
    auto p = ::operator new(1);
    ::operator delete(p);
}
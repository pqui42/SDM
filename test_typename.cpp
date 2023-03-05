#include "typename.h"
#include <gtest/gtest.h>


TEST(GetTypeName, success) {
    bool typeCheck = std::string("bool") == GetTypeName<bool>();
    assert(typeCheck);  
}

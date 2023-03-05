#include <cassert>
#include <map>
#include "palo.h"

int main() {

    using Key = uint64_t;
    using T = uint64_t;
    using Compare = std::less<Key>;
    using Type = std::pair<const Key, T>;
    using GCCInternalTypeForAMapNode= std::_Rb_tree_node<Type>;
    using Map = std::map<Key, T, Compare, PAlo<Type>>;

    const size_t no_slots_map = 5; 
    BumpAlo<GCCInternalTypeForAMapNode>::Get().AddMemory(no_slots_map);
     
    Map m;
    for (T i = 1; i <= no_slots_map; ++i)
    {
        m[i] = i*i;
    }
    for (const auto& n : m) {
        assert(n.first*n.first == n.second);
    }
    Map::iterator begin2 = Map::iterator(m.begin());
    Map::iterator end2 = Map::iterator(m.end());
    Map m2(begin2, end2);
    for (const auto& n : m2) {
        assert(n.first*n.first == n.second);
    }

    BumpAlo<GCCInternalTypeForAMapNode>::Get().AddMemory(no_slots_map);
    
    Map::iterator begin3;
    Map::iterator end3;
    
    Map m3(begin2, end2);
    for (const auto& n : m3) {
        assert(n.first*n.first == n.second);
    }
    begin3 = m3.begin();
    end3 = m3.end();
    
    for(Map::iterator it = begin3; it != end3; ++it) {
        auto first = it->first;
        auto second = it->second;
        assert(first*first == second);
    } 
    
    
    Map::iterator begin4;
    Map::iterator end4;
    {
        Map m4;
        for (T i = 1; i <= no_slots_map; ++i)
        {
            m4[i+5] = (i+5)*(i+5);
        }
        for (const auto& n : m4) {
            assert(n.first*n.first == n.second);
        }
        begin4 = m4.begin();
        end4 = m4.end();
    }

    for(Map::iterator it = begin4; it != end4; ++it) {
        auto first = it->first;
        auto second = it->second;
        assert(first*first == second);
    } 

    Map per_e;

    per_e[1]= 42;
    per_e[42] = 1;
    assert(per_e[1] == 42);
    assert(per_e[42] == 1);   
}
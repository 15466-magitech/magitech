#pragma once

#include <cstdlib>
#include <unordered_map>

template<typename T>
struct Component {
    static std::unordered_map<uint32_t, T> &get_map() {
        static std::unordered_map<uint32_t, T> map;
        return map;
    };
};

#pragma once

#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <functional>
#include "Entity.hpp"

template<typename T>
struct Component {
    static std::unordered_map<uint32_t, T> &get_map() {
        static std::unordered_map<uint32_t, T> map;
        return map;
    };
    
    /* added to in Entity::remove_component<T>() */
    inline static std::vector<uint32_t> to_delete;
    inline static std::unordered_map<uint32_t, T> to_add;
    inline static bool system_running = false;
    
    /*
     * Run the given function on all components in arbitrary order.
     * Thanks to the addition of Component<T>::to_delete and Component<T>::system_running
     * and revision of Entity code, Entity::remove_component<T>() should be fine to be called in the
     * lambda passed to Component<T>::system(f) so long as no component is removed more than once.
     */
    static void system(const std::function<void(T &)> &f) {
        system_running = true;
        for (auto &[_, component]: get_map()) {
            f(component);
        }
        system_running = false;
        for (uint32_t id: to_delete) {
            get_map().erase(id);
        }
        to_delete.clear();
        for (auto [id, component]: to_add) {
            get_map().emplace(id, component);
        }
        to_add.clear();
    }
};

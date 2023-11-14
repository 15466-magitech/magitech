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
    
    // added to in Entity::remove_component<T>()
    inline static std::vector<Entity *> to_delete;
    inline static bool system_running = false;
    
    static void system(const std::function<void(T &)> &f) {
        system_running = true;
        for (auto &[_, component]: get_map()) {
            f(component);
        }
        system_running = false;
        for (Entity *entity: to_delete) {
            entity->remove_component<T>();
        }
        to_delete.clear();
    }
};

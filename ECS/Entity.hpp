/*
 * Created by Matei Budiu on 10/26/23.
 * Author(s): Matei Budiu, Russell Emerine, Nellie Tonev
 */
#pragma once

#include <cstdlib>
#include <unordered_map>
#include <typeindex>
#include <iostream>
#include <functional>

struct Entity {
    Entity();
    
    /*
     * Destruct the entity by removing all components that haven't yet been removed
     */
    ~Entity();
    
    /*
     * Unique identity for querying and deleting entities, generated uniquely in the constructor
     */
    uint32_t id;
    
    /*
     * Associate a component with this entity, update map of entities with component T
     */
    template<typename T, typename... Args>
    T &add_component(Args &&... args) {
        to_delete[std::type_index(typeid(T))] = [this]() {
            // I don't just call remove_component because that invalidates the iterator in ~Entity
            std::unordered_map<uint32_t, T> &entity_to_component = T::get_map();
            entity_to_component.erase(id);
        };
        
        std::unordered_map<uint32_t, T> &entity_to_component = T::get_map();
        // this call to emplace looks weird but I'm pretty sure it's correct
        return entity_to_component.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(args...)
        ).first->second;
    }
    
    /*
     * Look up a component associated with this entity
     */
    template<typename T>
    T *get_component() {
        std::unordered_map<uint32_t, T> &entity_to_component = T::get_map();
        auto f = entity_to_component.find(id);
        if (f == entity_to_component.end()) return nullptr;
        else return &f->second;
    }
    
    /*
     * Remove a component associated with this entity (if the entity has the component)
     */
    template<typename T>
    void remove_component() {
        if (T::system_running) {
            T::to_delete.push_back(this);
        } else {
            to_delete.erase(std::type_index(typeid(T)));
            std::unordered_map<uint32_t, T> &entity_to_component = T::get_map();
            entity_to_component.erase(id);
        }
    }

private:
    /*
     * Keep track of what components to delete upon destruction
     */
    std::unordered_map<std::type_index, std::function<void()>> to_delete;
};

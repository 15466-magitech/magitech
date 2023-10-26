//
// Created by Matei Budiu on 10/26/23.
//

#ifndef MAGITECH_ECS_H
#define MAGITECH_ECS_H
#include <unordered_set>

struct Entity;

struct Component {
    std::unordered_set<Entity*> entities;
};

struct Entity {
    std::unordered_set<Component*> components;

    void addComponent(Component* c) {
        components.insert(c);
        c->entities.insert(this);
    }

    ~Entity() {
        for (Component* c: components) {
            c->entities.erase(this);
        }
    }
};


#endif //MAGITECH_ECS_H

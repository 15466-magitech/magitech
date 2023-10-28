/*
 * Created by Matei Budiu on 10/26/23.
 * Author(s): Nellie Tonev
 *
 * Adapted from Entity code covered in class on 10/24/23.
 */

#include "Entity.hpp"

Entity::Entity() {
    /* create unique id for entity lookup */
    static uint32_t unique_id = 0;
    id = unique_id;
    unique_id++;
}

Entity::~Entity() {
    for (auto [_, f]: to_delete) {
        std::cout << "deleting " << _.name() << "\n";
        f();
    }
}

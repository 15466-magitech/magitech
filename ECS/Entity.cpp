/*
 * Created by Matei Budiu on 10/26/23.
 * Author(s): Nellie Tonev
 *
 * Adapted from Entity code covered in class on 10/24/23.
 */

#include "Entity.hpp"

#include <cstdlib>
#include <unordered_map>

/* Component imports */
#include "Components/EventHandler.hpp"

Entity::Entity() {
	/* create unique id for entity lookup */
	static uint32_t unique_id = 0;
	id = unique_id;
	unique_id++;
}

template< typename T >
T &Entity::add_component() {
	std::unordered_map< uint32_t , T > &entity_to_component = T::get_map();
	return entity_to_component.emplace(id, T()).first->second;
}

template< typename T >
T *Entity::get_component() {
	std::unordered_map< uint32_t , T > &entity_to_component = T::get_map();
	auto f = entity_to_component.find(id);
	if (f == entity_to_component.end()) return nullptr;
	else return &f->second;
}

template< typename T >
void Entity::remove_component() {
	std::unordered_map< uint32_t , T > &entity_to_component = T::get_map();
	entity_to_component.erase(id);
}

/* destructor will remove all components from entity */
Entity::~Entity() {
	// TODO: import all headers from Components folder and call remove_component<Component>
	remove_component<EventHandler>();
}

/*
 * Created by Matei Budiu on 10/26/23.
 * Author(s): Matei Budiu, Russell Emerine, Nellie Tonev
 */
#pragma once

#include <cstdlib>
#include <unordered_map>

struct Entity {
	Entity();
	~Entity();

	uint32_t id; // unique identity for querying and deleting entities

	/* associate a component with this entity, update map of entities with component T */
	template< typename T >
	T &add_component();

	/* look up a component associated with this entity */
	template< typename T >
	T *get_component();

	/* remove a component associated with this entity (if the entity has the component) */
	template< typename T >
	void remove_component();
};

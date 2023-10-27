/*
 * Created by Nellie Tonev on 10/27/23.
 * Author(s): Nellie Tonev
 *
 * Component that corresponds to PlayMode::handle_event
 */
#pragma once

#include <cstdlib>
#include <unordered_map>

#include "PlayMode.hpp"

struct EventHandler {
	EventHandler() = default;

	static std::unordered_map< uint32_t , EventHandler > &get_map() {
		static std::unordered_map< uint32_t , EventHandler > map;
		return map;
	}

	/* when you add this component to an entity, need to replace it with an appropriate handle_event function */
	std::function< bool(SDL_Event const &, glm::uvec2 const &) > handle_event = nullptr;
};

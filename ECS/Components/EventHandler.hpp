/*
 * Created by Nellie Tonev on 10/27/23.
 * Author(s): Nellie Tonev, Russell Emerine
 *
 * Component that corresponds to PlayMode::handle_event
 */
#pragma once

#include <cstdlib>
#include <unordered_map>

#include <SDL.h>
#include <glm/glm.hpp>
#include "../HandlerComponent.hpp"

/* A struct to handle SDL input events */
struct EventHandler : HandlerComponent<EventHandler, bool, SDL_Event const &, glm::uvec2 const &> {
    using HandlerComponent<EventHandler, bool, SDL_Event const &, glm::uvec2 const &>::HandlerComponent;
    
    /* Handle all events in no particular order. Returns true if any handler handles the event. */
    static bool handle_event_all(SDL_Event const &evt, glm::uvec2 const &window_size);
};

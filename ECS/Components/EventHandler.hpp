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
#include "../Component.hpp"

struct EventHandler : Component<EventHandler> {
    explicit EventHandler(const std::function<bool(SDL_Event const &, glm::uvec2 const &)> &f);
    
    bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size);

private:
    const std::function<bool(SDL_Event const &, glm::uvec2 const &)> &handler;
};

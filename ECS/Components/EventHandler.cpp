#include "EventHandler.hpp"

EventHandler::EventHandler(const std::function<bool(const SDL_Event &, const glm::uvec2 &)> &f)
        : handler(f) {}

bool EventHandler::handle_event(const SDL_Event &evt, const glm::uvec2 &move) {
    return handler(evt, move);
}

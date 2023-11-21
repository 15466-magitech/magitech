#include <functional>
#include "EventHandler.hpp"

bool EventHandler::handle_event_all(const SDL_Event &evt, const glm::uvec2 &window_size) {
    bool handled;
    EventHandler::system([&](EventHandler &h) {
        handled |= h.handle(evt, window_size);
    });
    return handled;
}

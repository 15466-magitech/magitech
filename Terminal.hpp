#pragma once

#include <SDL.h>

#include "TextDisplay.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TerminalCommandHandler.hpp"

/*
 * An on-screen Terminal
 */
struct Terminal : Entity {
    TextDisplay text_display;
    
    /*
     * Construct a Terminal of as many rows and columns of characters
     * at the specified location on screen (bottom left corner, coordinates in [-1, 1] x [-1, 1])
     * with the specified display size (as fraction of [-1, 1] x [-1, 1]).
     * Starts out unactivated
     */
    Terminal(size_t rows, size_t cols, glm::vec2 loc, glm::vec2 size);
    
    /*
     * Activate the terminal. This adds the EventHandler component and activates the text display.
     * EventHandler in turn will call the TerminalCommandHandler trigger on commands.
     */
    void activate();
    
    /*
     * Deactivate the terminal. This removes the EventHandler component, and deactivates the text display,
     * and also calls the TerminalDeactivateHandler trigger.
     */
    void deactivate();
    
    /*
     * Handle a key event (used internally by `activate`)
     */
    bool handle_key(SDL_Keycode key);
};

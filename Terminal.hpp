#pragma once

#include <SDL.h>

#include "ECS/Entity.hpp"
#include "MonospaceFont.hpp"

/*
 * An on-screen Terminal
 */
struct Terminal : Entity {
    MonospaceFont font;
    
    bool active = false;
    
    std::vector<std::string> text = {""};
    
    size_t rows, cols;
    glm::vec2 loc, size;
    
    /*
     * Construct a Terminal of as many rows and columns of characters
     * at the specified location on screen (bottom left corner, coordinates in [-1, 1] x [-1, 1])
     * with the specified display size (as fraction of [-1, 1] x [-1, 1]).
     */
    Terminal(size_t rows, size_t cols, glm::vec2 loc, glm::vec2 size);
    
    void activate();
    
    void deactivate();
    
    /*
     * Handle a key event
     */
    bool handle_key(SDL_Keycode key);
    
    /*
     * Draw the Terminal. This should come last, after other drawing.
     */
    void draw();
};

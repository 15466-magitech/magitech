#pragma once

#include <SDL.h>

#include "ECS/Entity.hpp"
#include "MonospaceFont.hpp"
#include "ECS/Component.hpp"
#include "ECS/Components/TerminalCommandHandler.hpp"

/*
 * An on-screen text display
 */
struct TextDisplay : Entity {
    MonospaceFont font;
    
    std::vector<std::string> text = {""};
    
    size_t rows, cols;
    glm::vec2 loc, size;
    
    /*
     * Construct a text display of as many rows and columns of characters
     * at the specified location on screen (bottom left corner, coordinates in [-1, 1] x [-1, 1])
     * with the specified display size (as fraction of [-1, 1] x [-1, 1]).
     * Starts out unactivated
     */
    TextDisplay(size_t rows, size_t cols, glm::vec2 loc, glm::vec2 size);
    
    /*
     * Activates the text display, which means adding the Draw component
     */
    void activate();
    
    void deactivate();
    
    /*
     * Add some string in the terminal
     * Need to make sure every std::string can fit in one line, there is no check in the function
     */
    void add_text(const std::vector<std::string>&);
};

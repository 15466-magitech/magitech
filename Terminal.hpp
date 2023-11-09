#pragma once

#include <SDL.h>

#include "ECS/Entity.hpp"
#include "MonospaceFont.hpp"

/*
 * The possible commands that can be entered into the terminal
 */
enum struct Command {
    False = 0, // only falsey value, means handle_key did not process the input
    True, // means handle_key did process the input but there's nothing else interesting to report
    OpenSesame,
    Mirage
};

// TODO: in the future, maybe make "reacts to a terminal command" be a component and have the Terminal call a related system

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
    Command handle_key(SDL_Keycode key);
    
    /*
     * Draw the Terminal. This should come last, after other drawing.
     */
    void draw();

    /*
     * Add some string in the terminal
     * Need to make sure every std::string can fit in one line, there is no check in the function
     */
    void add_text(std::vector<std::string>);
};

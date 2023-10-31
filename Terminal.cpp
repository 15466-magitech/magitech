//
// Created by Russell Emerine on 10/30/23.
//

#include "GL.hpp"
#include "Terminal.hpp"

Terminal::Terminal(size_t rows, size_t cols, glm::vec2 loc, glm::vec2 size)
        : font("UbuntuMono.png"), rows(rows), cols(cols), loc(loc), size(size) {
    assert(rows > 0);
    assert(cols > 0);
    
    /*
    This should be here but for now there isn't a way to guarantee that it comes last.
    
    add_component<EventHandler>([this](const SDL_Event &evt, const glm::uvec2 &window_size) {
        if (evt.type == SDL_KEYDOWN) {
            return handle_key(evt.key.keysym.sym);
        } else {
            return false;
        }
    });
     */
}

void Terminal::activate() {
    active = true;
}

void Terminal::deactivate() {
    active = false;
}

bool Terminal::handle_key(SDL_Keycode key) {
    if (!active) return false;
    
    std::string keyname(SDL_GetKeyName(key));
    std::locale locale("C");
    if (key == SDLK_ESCAPE) {
        deactivate();
        return true;
    } else if (key == SDLK_BACKSPACE) {
        if (!text.empty() && !text.back().empty()) {
            text.back().pop_back();
        }
        return true;
    } else if (key == SDLK_RETURN) {
        text.emplace_back();
        if (text.size() > rows) {
            text.erase(text.begin());
        }
        return true;
    } else if (key == SDLK_SPACE) {
        char c = ' ';
        if (!text.empty() && text.back().size() < cols) {
            text.back().push_back(c);
        }
        return true;
    } else if (keyname.size() == 1 && std::isgraph(keyname[0], locale)) {
        char c = std::tolower(keyname[0], locale);
        if (!text.empty() && text.back().size() < cols) {
            text.back().push_back(c);
        }
        return true;
    }
    
    return false;
}

void Terminal::draw() {
    if (!active) return;
    
    // lol this works
    font.draw(' ', loc, size);
    
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            char c = ' ';
            if (row < text.size() && col < text[row].size()) {
                c = text[row][col];
            }
            auto char_size = glm::vec2(size.x / (float) cols, size.y / (float) rows);
            font.draw(
                    c,
                    glm::vec2(
                            loc.x + (float) col * char_size.x,
                            loc.y + size.y - (float) row * char_size.y
                    ),
                    char_size
            );
        }
    }
}

//
// Created by Russell Emerine on 11/20/23.
//

#include "TextDisplay.hpp"
#include "ECS/Components/Draw.hpp"

TextDisplay::TextDisplay(size_t rows, size_t cols, glm::vec2 loc, glm::vec2 size)
        : font("UbuntuMono.png"), rows(rows), cols(cols), loc(loc), size(size), _is_activated(false) {
    assert(rows > 0);
    assert(cols > 0);
}

void TextDisplay::activate() {
    add_component<Draw>([this]() {
        // a stand-in for the background (possibly removable)
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
    });

    _is_activated = true;
}

void TextDisplay::deactivate() {
    remove_component<Draw>();
    _is_activated = false;
}

void TextDisplay::add_text(const std::vector<std::string> &strs) {
    std::vector<std::string> fitted_strs;
    
    for (const auto &v: strs) {
        auto v_copy = v;
        while (v_copy.size() > cols) {
            auto pos = v_copy.rfind(' ', cols);
            
            auto shortened_string = pos != std::string::npos ?
                                    std::string(v_copy.begin(), v_copy.begin() + pos) :
                                    std::string(v_copy.begin(), v_copy.end());
            fitted_strs.push_back(shortened_string);
            
            v_copy = pos != std::string::npos ?
                     std::string(v_copy.begin() + pos, v_copy.end()) :
                     std::string(v_copy.end(), v_copy.end());
        }
        
        fitted_strs.push_back(v_copy);
    }
    
    
    for (const auto &str: fitted_strs) {
        text.push_back(str);
    }
    
    while (text.size() > rows) {
        text.erase(text.begin());
    }
}


bool TextDisplay::is_activated(){
    return _is_activated;
}
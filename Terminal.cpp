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

void Terminal::add_text(std::vector<std::string> strs){
    std::vector<std::string> fitted_strs;
    
    for(auto v : strs){
        auto v_copy = v;
        while(v_copy.size() > cols){
            auto loc = v_copy.rfind(" ", cols);

            auto shortened_string = loc != std::string::npos ? 
            std::string(v_copy.begin(),v_copy.begin() + loc) :
            std::string(v_copy.begin(),v_copy.end());
            fitted_strs.push_back(shortened_string);

            v_copy = loc != std::string::npos ? 
            std::string(v_copy.begin() + loc,v_copy.end()):
            std::string(v_copy.end(),v_copy.end());
        }

        fitted_strs.push_back(v_copy);
    }


    for(auto str : fitted_strs){
        text.push_back(str);
    }

    while(text.size() > rows){
        text.erase(text.begin());
    }
}

Command Terminal::handle_key(SDL_Keycode key) {
    if (!active) return Command::False;
    
    std::string keyname(SDL_GetKeyName(key));
    std::locale locale("C");
    if (key == SDLK_ESCAPE) {
        deactivate();
        return Command::True;
    } else if (key == SDLK_BACKSPACE) {
        if (!text.empty() && !text.back().empty()) {
            text.back().pop_back();
        }
        return Command::True;
    } else if (key == SDLK_RETURN) {
        Command command = Command::True;
        if (!text.empty()) {
            if (text.back() == "open sesame") {
                command = Command::OpenSesame;
                text.emplace_back("opening...");
                if (text.size() > rows) {
                    text.erase(text.begin());
                }
            } else if (text.back() == "mirage") {
                command = Command::Mirage;
                text.emplace_back("activating illusion magic...");
                if (text.size() > rows) {
                    text.erase(text.begin());
                }
            } else if (!text.back().empty()) {
                text.emplace_back("invalid command");
                if (text.size() > rows) {
                    text.erase(text.begin());
                }
            }
        }
        
        text.emplace_back();
        if (text.size() > rows) {
            text.erase(text.begin());
        }
        return command;
    } else if (key == SDLK_SPACE) {
        char c = ' ';
        if (!text.empty() && text.back().size() < cols) {
            text.back().push_back(c);
        }
        return Command::True;
    } else if (keyname.size() == 1 && std::isgraph(keyname[0], locale)) {
        char c = std::tolower(keyname[0], locale);
        if (!text.empty() && text.back().size() < cols) {
            text.back().push_back(c);
        }
        return Command::True;
    }
    
    return Command::False;
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

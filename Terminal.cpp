//
// Created by Russell Emerine on 10/30/23.
//

#include <locale>

#include "Terminal.hpp"
#include "ECS/Components/EventHandler.hpp"
#include "ECS/Components/TerminalDeactivateHandler.hpp"

Terminal::Terminal(size_t rows, size_t cols, glm::vec2 loc, glm::vec2 size)
        : text_display(rows, cols, loc, size) {}

void Terminal::activate() {
    add_component<EventHandler>([this](const SDL_Event &evt, const glm::uvec2 &window_size) {
        if (evt.type == SDL_KEYDOWN) {
            return handle_key(evt.key.keysym.sym);
        } else {
            return false;
        }
    });
    text_display.activate();
}

void Terminal::deactivate() {
    remove_component<EventHandler>();
    text_display.deactivate();
    TerminalDeactivateHandler::handle_all();
}

bool Terminal::handle_key(SDL_Keycode key) {
    std::string keyname(SDL_GetKeyName(key));
    std::locale locale("C");
    if (key == SDLK_ESCAPE) {
        deactivate();
        return true;
    } else if (key == SDLK_BACKSPACE) {
        if (!text_display.text.empty() && !text_display.text.back().empty()) {
            text_display.text.back().pop_back();
        }
        return true;
    } else if (key == SDLK_RETURN) {
        if (!text_display.text.empty()) {
            if (text_display.text.back() == "open sesame") {
                text_display.text.emplace_back("opening...");
                if (text_display.text.size() > text_display.rows) {
                    text_display.text.erase(text_display.text.begin());
                }
                TerminalCommandHandler::handle_all(Command::OpenSesame);
            } else if (text_display.text.back() == "mirage") {
                text_display.text.emplace_back("activating illusion magic...");
                if (text_display.text.size() > text_display.rows) {
                    text_display.text.erase(text_display.text.begin());
                }
                TerminalCommandHandler::handle_all(Command::Mirage);
            } else if (text_display.text.back() == "cook") {
                text_display.text.emplace_back("making a dish...");
                if (text_display.text.size() > text_display.rows) {
                    text_display.text.erase(text_display.text.begin());
                }
                TerminalCommandHandler::handle_all(Command::Cook);
            } else if (!text_display.text.back().empty()) {
                text_display.text.emplace_back("invalid command");
                if (text_display.text.size() > text_display.rows) {
                    text_display.text.erase(text_display.text.begin());
                }
            }
        }
        
        text_display.text.emplace_back();
        if (text_display.text.size() > text_display.rows) {
            text_display.text.erase(text_display.text.begin());
        }
        return true;
    } else if (key == SDLK_SPACE) {
        char c = ' ';
        if (!text_display.text.empty() && text_display.text.back().size() < text_display.cols) {
            text_display.text.back().push_back(c);
        }
        return true;
    } else if (keyname.size() == 1 && std::isgraph(keyname[0], locale)) {
        char c = std::tolower(keyname[0], locale);
        if (!text_display.text.empty() && text_display.text.back().size() < text_display.cols) {
            text_display.text.back().push_back(c);
        }
        return true;
    }
    
    return false;
}

#include "PauseMode.hpp"

#include "ShadowProgram.hpp"
#include "ComicBookProgram.hpp"
#include "RocketColorTextureProgram.hpp"
#include "TextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "TextStorage.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Components/EventHandler.hpp"
#include "ECS/Components/TerminalDeactivateHandler.hpp"
#include "ECS/Components/Draw.hpp"
#include "spline.h"
#include "TexProgram.hpp"
#include "load_save_png.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <utility>


PauseMode::PauseMode(SDL_Window *window)
        : sign_display(5, 40, glm::vec2(-0.3f,-0.25f), glm::vec2(0.6f,0.4f),"UbuntuMono.png"),
          text_display(13, 70, glm::vec2(-0.5f, -0.5f), glm::vec2(1.0f, 1.0f)),
          terminal(10, 30, glm::vec2(0.05f, 0.05f), glm::vec2(0.4f, 0.4f)) {
    
    this->window = window;
    

    // this activates the player component stuff
    terminal.activate();
    terminal.deactivate();
    text_display.activate();
    text_display.deactivate();
}

PauseMode::~PauseMode() = default;

bool PauseMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

    if (evt.key.keysym.sym == SDLK_BACKQUOTE) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        return true;
    }


	return false;
}

void PauseMode::update(float elapsed) {

}




void PauseMode::draw(glm::uvec2 const &drawable_size) {
    switch (game_state)
    {
    case START:
    {
        text_display.remove_all_text();
        text_display.add_text(
            std::vector<std::string>{
                "",
                "",
                "",
                "Welcome to TechWiz",
                "",
                "S : press S to start the game",
                "Q in menu : press Q to exit the game",
                "Q in game : press Q to pause the game"
                //"` : press backquote to detach mouse"
            }
        );
        text_display.activate();
        break;
    }

    case PAUSE:
    {
        text_display.remove_all_text();
        text_display.add_text(
            std::vector<std::string>{
                "",
                "",
                "",
                "Game is paused",
                "",
                "Q : press Q to exit the game",
                "ESC : press ESC to continue the game"
                //"` : press backquote to detach mouse"
            }
        );
        text_display.activate();
        break;
    }

    case END:
    {
        text_display.remove_all_text();
        text_display.add_text(
            std::vector<std::string>{
                "",
                "The journey of TechWiz has come to an end",
                "",
                "Q : press Q to exit the game",
                "",
                "",
                "Created by Matei Budiu, Nellie Tonev, Russel Emerine, Michael Stroucken and Yuan Meng",
                "",
                "Thank you!"

            }
        );
        text_display.activate();
        break;
    }

    
    default:
        break;
    }


    Draw::handle_all();
    text_display.deactivate();
}




#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "Terminal.hpp"
#include "spline.h"
#include "load_save_png.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <chrono>






struct PauseMode : Mode {
    PauseMode(SDL_Window* window);
    
    ~PauseMode() override;
    
    //functions called by main loop:
    bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;



    void update(float elapsed) override;
    
    void draw(glm::uvec2 const &drawable_size) override;
    
    //----- game state -----
    TextDisplay sign_display;
    TextDisplay text_display;
    Terminal terminal;
    
    //input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, down, up, esc, read, run;

    SDL_Window* window;
    //local copy of the game scene (so code can change it during gameplay):



    //Scene change
    decltype(std::chrono::system_clock::now()) start_timepoint;
    decltype(std::chrono::system_clock::now()) end_timepoint;
    bool is_changing_scene = false;

};

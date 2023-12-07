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
    explicit PauseMode(SDL_Window* window);
    
    ~PauseMode() override;
    
    //functions called by main loop:
    bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;



    void update(float elapsed) override;
    
    void draw(glm::uvec2 const &drawable_size) override;
    
    //----- game state -----
    TextDisplay text_display;
    
    //input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, down, up, esc, read, run;

    SDL_Window* window;
};

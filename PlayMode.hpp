#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Terminal.hpp"
#include "spline.h"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
    PlayMode();
    
    ~PlayMode() override;
    
    //functions called by main loop:
    bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
    
    void update(float elapsed) override;
    
    void draw(glm::uvec2 const &drawable_size) override;
    
    //----- game state -----
    
    Terminal terminal;
    
    //input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, down, up, read, use;
    // camera animation
    bool animated = false;
    float animationTime = 0.0f;
    Spline<glm::vec3> splineposition, splinerotation;
    
    //local copy of the game scene (so code can change it during gameplay):
    Scene scene;
    
    //player info:
    struct Player {
        WalkPoint at;
        //transform is at player's feet and will be yawed by mouse left/right motion:
        Scene::Transform *transform = nullptr;
        //camera is at player's head and will be pitched by mouse up/down motion:
        Scene::Camera *camera = nullptr;

        //other metadata
        std::string name;
    } player;

    // Wireframe logics
	std::list<std::shared_ptr<Scene::Collider>> wireframe_objects;
	std::unordered_map<std::string, std::shared_ptr<Scene::Collider>> current_wireframe_objects_map;
	void update_wireframe();
};

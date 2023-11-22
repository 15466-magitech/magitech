#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "Terminal.hpp"
#include "spline.h"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
    PlayMode(SDL_Window* window);
    
    ~PlayMode() override;
    
    //functions called by main loop:
    bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
    
    void update(float elapsed) override;
    
    void draw(glm::uvec2 const &drawable_size) override;
    
    //----- game state -----
    
    Terminal terminal, text_display;
    
    //input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, down, up, read;
    // camera animation
    bool animated = false;
    float animationTime = 0.0f;
    Spline<glm::vec3> splineposition;
    Spline<glm::quat> splinerotation;

    GLuint depth_fb;
    GLuint depth_tex;

    SDL_Window* window;
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
        std::string name = "Player";
    } player;
    
    // Wireframe logics
    bool has_paint_ability = false;
    std::list<std::shared_ptr<Scene::Collider>> wireframe_objects;
    std::unordered_map<std::string, std::shared_ptr<Scene::Collider>> current_wireframe_objects_map;
    //std::list<std::shared_ptr<Scene::Collider>> wf_obj_pass; // Object on walkmesh, blocked by invisible bbox when it's wireframe
    std::unordered_map<std::string, std::shared_ptr<Scene::Collider>> wf_obj_pass_map;
    //std::list<std::shared_ptr<Scene::Collider>> wf_obj_block; // Normal object, blocked when it's real by bbox
    std::unordered_map<std::string, std::shared_ptr<Scene::Collider>> wf_obj_block_map;


    //debug
    std::list<std::pair<glm::vec3,glm::vec3>> rays;
    //

    void genFramebuffers();
    void resizeDepthTex();

    void update_wireframe();
    void update_wireframe(std::shared_ptr<Scene::Collider> collider);
    
    void initialize_wireframe_objects(std::string prefix);
    
    
    // Unlock logics(for open sesame)
    void unlock(std::string prefix);
    
    
    //initilization functions
    void initialize_scene_metadata();
    
    void initialize_collider(std::string prefix_pattern, Load<MeshBuffer> meshes);

    void initialize_text_collider(std::string prefix_pattern, Load<MeshBuffer> meshes);

    // Mouse-collider check return the collider and the distance pair
    std::pair<std::shared_ptr<Scene::Collider>,float> mouse_collider_check(std::string prefix="col_",bool use_crosshair = false);
    std::pair<std::shared_ptr<Scene::Collider>,float> mouse_text_check(std::string prefix="text_",bool use_crosshair = false);
};

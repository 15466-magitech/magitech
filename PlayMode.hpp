#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Terminal.hpp"
#include "Load.hpp"
#include "Mesh.hpp"

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
    } left, right, down, up , use;
    
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
	void update_wireframe();
    void initialize_wireframe_objects(std::string prefix);



    //initilization functions
    void initialize_scene_metadata();
    void initialize_collider(std::string prefix_pattern, Load<MeshBuffer> meshes);
};

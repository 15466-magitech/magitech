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


typedef enum{
    WIREFRAME,
    DOOR,
    FOOD,
    UNKNOWN
} ColliderType;

typedef enum {
  NO = 0,
  TO = 1,
  THERE = 2,
  FROM = 3,
} animation_t;

struct PlayMode : Mode {
    PlayMode(SDL_Window* window);
    
    ~PlayMode() override;
    
    //functions called by main loop:
    bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
    
    void update(float elapsed) override;
    
    void draw(glm::uvec2 const &drawable_size) override;
    
    //----- game state -----
    
    TextDisplay text_display;
    Terminal terminal;
    
    //input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, down, up, esc, read;
    // camera animation
    animation_t animated = NO;
    float animationTime = 0.0f;
    Spline<glm::vec3> splineposition;
    Spline<glm::quat> splinerotation;

    GLuint depth_fb;
    GLuint depth_tex;
    GLuint dot_tex;
    GLuint R_tex;

    GLuint shadow_depth_fb;
    GLuint shadow_depth_tex;

    SDL_Window* window;
    //local copy of the game scene (so code can change it during gameplay):
    std::shared_ptr<Scene> scene;
    std::map<scene_type,std::shared_ptr<Scene>> scene_map;
    scene_type current_scene_type;
    
    //player info:
    struct Player : Entity {
        WalkPoint at;
        //transform is at player's feet and will be yawed by mouse left/right motion:
        Scene::Transform *transform = nullptr;
        //camera is at player's head and will be pitched by mouse up/down motion:
        Scene::Camera *camera = nullptr;
        
        //other metadata
        std::string name = "Player";

        //player ability
        bool has_paint_ability = false;
        bool has_unlock_ability = false;
        bool has_bounce_ability = true;

        static constexpr float SIGHT_DISTANCE = 5.0f;

        // camera positioning
        //default view point behind player
        // Due to the crosshair, need to move player a little left/right
        static constexpr glm::vec3 defaultCameraPosition = glm::vec3(-1.0f, -5.0f, 2.5f);
        //rotate camera to something pointing in way of player
        // arcsin 0.1 ~ 6 degrees
        static constexpr glm::vec3 defaultCameraRotation = glm::vec3(glm::radians(84.0f), glm::radians(0.0f), glm::radians(0.0f));

        // Bread bouncing logic
        bool on_walkmesh = true;
        Spline<glm::vec3> player_bounce_spline;
        float interpolation_time = 0.0f;
        uint8_t bounce_stage = 0; // 0 means not in bouce stage. 1 means from location to bread. 2 means from bread to destination
        glm::vec3 bounce_destination{0.0f};

    } player;

    void initialize_scene(Load<Scene>, Load<MeshBuffer>, scene_type);
    // Should be called after this->scene is not null
    void initialize_player();


    //debug
    std::list<std::pair<glm::vec3,glm::vec3>> rays;
    //

    void gen_dot_texture();
    void gen_framebuffers();
    void resize_depth_tex();
    void draw_black_screen();
    void draw_keyboard_sign(glm::vec3);
    void gen_R_texture();

    void update_wireframe();
    void update_wireframe(const std::shared_ptr<Scene::Collider>& collider);


    std::pair<std::string,glm::vec3> find_closest_sign();
    
    
    
    // Unlock logics(for open sesame)
    void unlock(const std::string& prefix);
    
    

    // Mouse-collider check return the collider and the distance pair
    std::pair<std::shared_ptr<Scene::Collider>,float> mouse_collider_check(const std::string& prefix="col_",bool use_crosshair = false);
    std::pair<std::shared_ptr<Scene::Collider>,float> mouse_text_check(const std::string& prefix="text_",bool use_crosshair = false);
    std::pair<std::shared_ptr<Scene::Collider>,float> mouse_bread_check(const std::string& prefix="bread_",bool use_crosshair = false);
    ColliderType check_collider_type(std::shared_ptr<Scene::Collider> c);


    // Bouncing logic
    void get_off_walkmesh();
    void set_bouncing_spline(glm::vec3);


    //Scene change
    decltype(std::chrono::system_clock::now()) start_timepoint;
    decltype(std::chrono::system_clock::now()) end_timepoint;
    bool is_changing_scene;

};

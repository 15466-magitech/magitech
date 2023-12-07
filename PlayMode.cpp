#include "PlayMode.hpp"

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

GLuint artworld_meshes_for_lit_color_texture_program = 0;
GLuint artworld_meshes_for_rocket_color_texture_program = 0;
GLuint foodworld_meshes_for_lit_color_texture_program = 0;
GLuint foodworld_meshes_for_rocket_color_texture_program = 0;
GLuint textcube_meshes_for_lit_color_texture_program = 0;
GLuint wizard_meshes_for_lit_color_texture_program = 0;
GLuint image_vao = 0;

// object name to transform
std::unordered_map<std::string, Scene::Transform *> nameToTransform;
// text bearer name to mesh
std::unordered_map<std::string, Mesh const *> textBearers;
// text bearer name to camera
std::unordered_map<std::string, std::string> textBearerCams;

// c++ still sucks
bool endsWith(const std::string &str, const std::string &suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

Load< Sound::Sample > olas_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("olas.opus"));
});

Load< Sound::Sample > fire_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("fire.opus"));
});

Load< Sound::Sample > door_open_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("door_open.opus"));
});

Load< Sound::Sample > cast_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("cast.wav"));
});

Load< Sound::Sample > bounce_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("boing.opus"));
});

Load< Sound::Sample > walk_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("concrete-footsteps.opus"));
});

Load< Sound::Sample > walk_15x_sample(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("concrete-footsteps_speed_up.opus"));
});



Load<MeshBuffer> artworld_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("artworld.pnct"));
    artworld_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    artworld_meshes_for_rocket_color_texture_program = ret->make_vao_for_program(rocket_color_texture_program->program);
    
    // register text bearers and their cameras
    for (const auto &[name, mesh]: ret->meshes) {
        if (name.rfind("text_", 0) != std::string::npos) {
            std::cout << "Found sign: " << name << std::endl;
            if (!endsWith(name, "_m")) {
                std::cerr << "Sign mesh " << name << " doesn't end in _m" << std::endl;
            } else {
                textBearers[name] = &mesh;
                std::string camname = name;
                camname.back() = 'c';
                textBearerCams[name] = camname;
            }
        }
    }
    
    return ret;
});


Load<MeshBuffer> foodworld_meshes(LoadTagDefault,[]() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("foodworld.pnct"));
    foodworld_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    foodworld_meshes_for_rocket_color_texture_program = ret->make_vao_for_program(rocket_color_texture_program->program);

    // register text bearers and their cameras
    for (const auto &[name, mesh] : ret->meshes) {
      if (name.rfind("text_", 0) != std::string::npos) {
std::cout << "Found sign: " << name << std::endl;
        if (!endsWith(name, "_m")) {
          std::cerr << "Sign mesh " << name << " doesn't end in _m" << std::endl;
        } else {
          textBearers[name] = &mesh;
          std::string camname = name;
          camname.back() = 'c';
          textBearerCams[name] = camname;
        }
      }
    }

    return ret;
});



Load<MeshBuffer> wizard_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("wizard.pnct"));
    wizard_meshes_for_lit_color_texture_program = ret->make_vao_for_program(rocket_color_texture_program->program);
    return ret;
});

Load<TextStorage> text_storage(LoadTagDefault, []() -> TextStorage const * {
    TextStorage const *ret = new TextStorage(data_path("text_binary"));
    return ret;
});

Load<Scene> artworld_scene(LoadTagDefault, []() -> Scene const * {
    return new Scene(
            data_path("artworld.scene"),
            [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
                // keep transforms available
                nameToTransform[mesh_name] = transform;
                
                if (mesh_name == "Player")
                    return;
                
                Mesh const &mesh = artworld_meshes->lookup(mesh_name);
                
                scene.drawables.emplace_back(std::make_shared<Scene::Drawable>(transform));
                std::shared_ptr<Scene::Drawable> &drawable = scene.drawables.back();
                
                if (artworld_meshes->lookup_collection(mesh_name) == "Rocket") {
                    drawable->pipeline = rocket_color_texture_program_pipeline;
                    drawable->pipeline.vao = artworld_meshes_for_rocket_color_texture_program;
                    drawable->specular_info.shininess = 10.0;
                    drawable->specular_info.specular_brightness = glm::vec3(1.0f, 0.9f, 0.7f);
                } else {
                    drawable->pipeline = lit_color_texture_program_pipeline;
                    drawable->pipeline.vao = artworld_meshes_for_lit_color_texture_program;
                    drawable->specular_info.shininess = 10.0;
                }
                
                drawable->pipeline.type = mesh.type;
                drawable->pipeline.start = mesh.start;
                drawable->pipeline.count = mesh.count;
                drawable->wireframe_info.draw_frame = false;
                drawable->wireframe_info.one_time_change = false;
                drawable->scene_info.type = ARTSCENE;
            });
});



Load<Scene> foodworld_scene(LoadTagDefault,[]() -> Scene const * {
    return new Scene(
            data_path("foodworld.scene"),
            [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
                // keep transforms available
                nameToTransform[mesh_name] = transform;

                if (mesh_name == "Player")
                    return;
                
                Mesh const &mesh = foodworld_meshes->lookup(mesh_name);
                
                scene.drawables.emplace_back(std::make_shared<Scene::Drawable>(transform));
                std::shared_ptr<Scene::Drawable> &drawable = scene.drawables.back();
                
                if (foodworld_meshes->lookup_collection(mesh_name) == "Rocket") {
                    //drawable->pipeline = lit_color_texture_program_pipeline;
                    //drawable->pipeline.vao = artworld_meshes_for_lit_color_texture_program;
                    drawable->pipeline = rocket_color_texture_program_pipeline;
                    drawable->pipeline.vao = foodworld_meshes_for_rocket_color_texture_program;
                    drawable->specular_info.shininess = 10.0;
                } else if (foodworld_meshes->lookup_collection(mesh_name) == "bg_noshadow") {
                    drawable->pipeline = shadow_program_pipeline;
                    drawable->pipeline.vao = foodworld_meshes_for_lit_color_texture_program;
                    drawable->specular_info.shininess = 5.0;
                    drawable->specular_info.specular_brightness = glm::vec3(0.5f, 0.5f, 0.5f);
                    drawable->ignore_shadow = true;
                } else {
                    drawable->pipeline = shadow_program_pipeline;
                    drawable->pipeline.vao = foodworld_meshes_for_lit_color_texture_program;
                    drawable->specular_info.shininess = 10.0;
                }
                
                drawable->pipeline.type = mesh.type;
                drawable->pipeline.start = mesh.start;
                drawable->pipeline.count = mesh.count;
                drawable->wireframe_info.draw_frame = false;
                drawable->wireframe_info.one_time_change = false;
                drawable->scene_info.type = FOODSCENE;
            });
});

WalkMesh const *walkmesh = nullptr;
WalkMesh const *artworld_walkmesh = nullptr;
Load<WalkMeshes> artworld_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
    auto *ret = new WalkMeshes(data_path("artworld.w"));
    artworld_walkmesh = &ret->lookup("WalkMesh");
    return ret;
});

WalkMesh const *foodworld_walkmesh = nullptr;
Load<WalkMeshes> foodworld_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
    auto *ret = new WalkMeshes(data_path("foodworld.w"));
    foodworld_walkmesh = &ret->lookup("WalkMesh");
    return ret;
});

PlayMode::PlayMode(SDL_Window *window)
        : sign_display(8, 40, glm::vec2(-0.3f,-0.25f), glm::vec2(0.6f,0.4f),"UbuntuMono_transparent_white.png"),
          text_display(5, 60, glm::vec2(-0.40f, -0.45f), glm::vec2(0.8f, 0.2f)),
          terminal(10, 30, glm::vec2(0.05f, 0.05f), glm::vec2(0.4f, 0.4f),"UbuntuMono_terminal.png") {
    
    this->window = window;
    glGenVertexArrays(1, &image_vao); 
    gen_framebuffers();
    image_vao = gen_image(glm::vec2(-1.0f, -1.0f), glm::vec2(2.0f, 2.0f), 0.0f, 0.0f, 1.0f, 1.0f);

    gen_R_texture();
    
    // TODO: remove this test code
    {
        std::cout << "Testing basic ECS mechanics..." << std::endl;
        struct TestComponent : Component<TestComponent> {
            std::string name;
            
            explicit TestComponent(std::string name) : name(std::move(name)) {}
        };
        Entity a, b, c, d;
        a.add_component<TestComponent>("A");
        b.add_component<TestComponent>("B");
        c.add_component<TestComponent>("C");
        d.add_component<TestComponent>("D");
        std::cout << "TestComponent A has name " << a.get_component<TestComponent>()->name << "\n";
        TestComponent::system([&d](TestComponent &x) {
            std::cout << "Hello from a TestComponent with name " << x.name << "!\n";
            if (x.name == "D") {
                std::cout << "Found a component with name D, deleting outside d...\n";
                d.remove_component<TestComponent>();
            }
        });
        c.remove_component<TestComponent>();
        TestComponent::system([](TestComponent &x) {
            std::cout << "Hello again from a TestComponent with name " << x.name << "!\n";
        });
        std::cout << "ECS ok" << std::endl;
    }
    {
        std::cout << "Testing spline" << std::endl;
        glm::vec2 start(2.0, 0.0);
        glm::vec2 end(0.0, 2.0);
        Spline<glm::vec2> spline;
        spline.set(0.0, start);
        spline.set(1.0, end);
        glm::vec2 query = spline.at(0.5);
        assert(query.x == 1.0);
        assert(query.y == 1.0);
        std::cout << "spline ok" << std::endl;
    }


    initialize_scene(artworld_scene,artworld_meshes,ARTSCENE);
    initialize_scene(foodworld_scene,foodworld_meshes,FOODSCENE);


    scene = scene_map[ARTSCENE];
    walkmesh = artworld_walkmesh;
    bgm = Sound::loop(*olas_sample);
    
    initialize_player();
    
    // this activates the player component stuff
    terminal.activate();
    terminal.deactivate();

    walk = Sound::loop(*walk_sample);
    walk_15x = Sound::loop(*walk_15x_sample);

    walk->set_volume(0.0f);
    walk_15x->set_volume(0.0f);


}

PlayMode::~PlayMode() = default;

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    return EventHandler::handle_event_all(evt, window_size);
}

void PlayMode::update(float elapsed) {
    if (animated == NO && read.pressed && animationTime == 0.0) {
        //float distance = std::numeric_limits<float>::max();
        float distance = player.SIGHT_DISTANCE; // can see this far
        auto playerToWorld = player.transform->make_local_to_world();
        auto here = player.transform->position;
        std::cout << "here " << here.x << " " << here.y << " " << here.z << std::endl;
        std::string selected;
        
        // find the closest text bearer
        for (const auto &[name, mesh]: textBearers) {
            if (!endsWith(name, "_m")) {
                continue;
            }
            auto transform = nameToTransform[name];
            // Is this correct???
            auto there_test = transform->make_local_to_world() * glm::vec4(transform->position, 1.0);
            auto tmp = transform->make_local_to_world();
            auto there = glm::vec3{tmp[3][0],tmp[3][1],tmp[3][2]};
            std::cout << "there " << there.x << " " << there.y << " " << there.z << std::endl;
            std::cout << "there_t " << there_test.x << " " << there_test.y << " " << there_test.z << std::endl;
            float newdistance = glm::distance(here, there);
            std::cout << "distance: " << newdistance << " name: " << name << std::endl;
            if (newdistance < distance) {
                distance = newdistance;
                selected = name;
            }
        }
        if (!selected.empty()) {
            std::cout << "selected: " << selected << std::endl;
            assert(selected.back() == 'm');
            std::string selectedCamera = textBearerCams[selected];
            auto destCamera = scene->cams[selectedCamera];
            assert(destCamera != nullptr);
            animated = TO;
            animationTime = 0.0f;
            auto selectedToWorld = nameToTransform[selected]->make_local_to_world();
            //auto playerCameraToWorld = player.camera->transform->make_local_to_world();
            auto startposition = playerToWorld * glm::vec4(player.camera->transform->position, 1.0);
            auto endposition = selectedToWorld * glm::vec4(destCamera->transform->position, 1.0);
            auto startrotation = glm::quat_cast(
                    glm::mat3(playerToWorld) * glm::mat3_cast(player.camera->transform->rotation));
            auto endrotation = glm::quat_cast(
                    glm::mat3(selectedToWorld) * glm::mat3_cast(destCamera->transform->rotation));
            splineposition = Spline<glm::vec3>();
            splinerotation = Spline<glm::quat>();
            splineposition.set(0.0f, startposition);
            splinerotation.set(0.0f, startrotation);
            splineposition.set(1.0f, endposition);
            splinerotation.set(1.0f, endrotation);
            // now use world camera
            player.camera->transform->parent = nullptr;
        } else {
            std::cout << "No readable sign in range" << std::endl;
        }
    }
    // camera animation
    if (animated == TO || animated == FROM) {
        animationTime += elapsed;
        animationTime = std::min(1.0f, animationTime);
        player.camera->transform->position = splineposition.at(animationTime);
        player.camera->transform->rotation = splinerotation.at(animationTime);
        if (animationTime == 1.0f) {
            animationTime = 0.0f;
            if (animated == TO) {
//std::cout << "arrived" << std::endl;
                animated = THERE;
                std::shared_ptr<Scene::Collider> c = nullptr;
                float distance = 0.0;
                std::tie(c, distance) = mouse_text_check("text_", true);
                if (c) {
                    if (c->name.find("text_sign_")!=std::string::npos){
                        if (text_storage->object_text_map.count(c->name)) {
                            auto v = text_storage->object_text_map.at(c->name);
                            sign_display.text = {""};
                            sign_display.add_text(v[0]);
                            sign_display.activate();
                        }
                    }
                    else if(c->name.find("text_col_npc_")!=std::string::npos){
                        if (text_storage->object_text_map.count(c->name)) {
                            auto v = text_storage->object_text_map.at(c->name);
                            text_display.text = {""};
                            for(auto tmp : v){
                                text_display.add_text(tmp);
                            }
                            text_display.activate();
                        }
                    }


                }
            } else {
                animated = NO;
                // back to player local camera
                player.camera->transform->position = player.defaultCameraPosition;
                player.camera->transform->rotation = player.defaultCameraRotation;
                player.camera->transform->parent = player.transform;
            }
        }
    }
    // reset camera
    if (animated == THERE && esc.pressed) {
//std::cout << "there" << std::endl;
        animated = FROM;
        animationTime = 0.0;
        auto playerToWorld = player.transform->make_local_to_world();
        splineposition.set(0.0f, player.camera->transform->position);
        splinerotation.set(0.0f, player.camera->transform->rotation);
        splineposition.set(1.0f, playerToWorld * glm::vec4(player.defaultCameraPosition, 1.0f));
        splinerotation.set(1.0f, glm::quat(
                glm::mat3(playerToWorld) * glm::mat3_cast(glm::quat(player.defaultCameraRotation))));
        sign_display.deactivate();
        sign_display.remove_all_text();
        text_display.deactivate();
        text_display.remove_all_text();
    }
    
    //player walking:
    {
        if(player.on_walkmesh){
            //combine inputs into a move:
            constexpr float PlayerSpeed = 3.0f;
            auto move = glm::vec2(0.0f);
            if (left.pressed && !right.pressed) move.x = -1.0f;
            if (!left.pressed && right.pressed) move.x = 1.0f;
            if (down.pressed && !up.pressed) move.y = -1.0f;
            if (!down.pressed && up.pressed) move.y = 1.0f;

            //make it so that moving diagonally doesn't go faster:
            if (move != glm::vec2(0.0f))
            {
                move = glm::normalize(move) * PlayerSpeed * elapsed;
                if (lrun.pressed || rrun.pressed) {
                    walk->set_volume(0.0f);
                    walk_15x->set_volume(1.0f);
                } else {
                    walk->set_volume(1.0f);
                    walk_15x->set_volume(0.0f);
                }

            } else {
                walk->set_volume(0.0f);
                walk_15x->set_volume(0.0f);
            }

            if (lrun.pressed || rrun.pressed)
            {
                move *= 2;
            } 

            //get move in world coordinate system:
            glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);
            
            //Collision
            {
                auto c = scene->collider_name_map[player.name];
                bool has_collision = false;
                
                
                // If there is collision, reverse the remain vector at the collision direction?
                int idx = -1;
                float overlap = std::numeric_limits<float>::infinity();
                
                
                for (const auto &collider: scene->colliders) {
                    if (collider->name == player.name) {
                        continue;
                    } else {
                        if (c->intersect(collider)) {
                            has_collision = true;
                            // Only one collision at a time?
                            std::tie(idx, overlap) = c->least_collison_axis(collider);
                            break;
                        }
                    }
                }
                
                if (has_collision) {
                    remain[idx] += overlap;
                }
            }
            
            //using a for() instead of a while() here so that if walkpoint gets stuck in
            // some awkward case, code will not infinite loop:
            for (uint32_t iter = 0; iter < 10; ++iter) {
                if (remain == glm::vec3(0.0f)) break;
                WalkPoint end;
                float time;
                walkmesh->walk_in_triangle(player.at, remain, &end, &time);
                player.at = end;
                if (time == 1.0f) {
                    //finished within triangle:
                    remain = glm::vec3(0.0f);
                    break;
                }
                //some step remains:
                remain *= (1.0f - time);
                //try to step over edge:
                glm::quat rotation;
                if (walkmesh->cross_edge(player.at, &end, &rotation)) {
                    //stepped to a new triangle:
                    player.at = end;
                    //rotate step to follow surface:
                    remain = rotation * remain;
                } else {
                    //ran into a wall, bounce / slide along it:
                    glm::vec3 const &a = walkmesh->vertices[player.at.indices.x];
                    glm::vec3 const &b = walkmesh->vertices[player.at.indices.y];
                    glm::vec3 const &c = walkmesh->vertices[player.at.indices.z];
                    glm::vec3 along = glm::normalize(b - a);
                    glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
                    glm::vec3 in = glm::cross(normal, along);
                    
                    //check how much 'remain' is pointing out of the triangle:
                    float d = glm::dot(remain, in);
                    if (d < 0.0f) {
                        //bounce off of the wall:
                        remain += (-1.25f * d) * in;
                    } else {
                        //if it's just pointing along the edge, bend slightly away from wall:
                        remain += 0.01f * d * in;
                    }
                }
            }
            
            if (remain != glm::vec3(0.0f)) {
                std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
            }
            
            //update player's position to respect walking:
            player.transform->position = walkmesh->to_world_point(player.at);
            
            { //update player's rotation to respect local (smooth) up-vector:
                
                glm::quat adjust = glm::rotation(
                        player.transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f), //current up vector
                        //walkmesh->to_world_smooth_normal(player.at) //smoothed up vector at walk location
                        glm::vec3(0.0, 0.0, 1.0)
                );
                player.transform->rotation = glm::normalize(adjust * player.transform->rotation);
            }
            
            /*
            glm::mat4x3 frame = camera->transform->make_local_to_parent();
            glm::vec3 right = frame[0];
            //glm::vec3 up = frame[1];
            glm::vec3 forward = -frame[2];

            camera->transform->position += move.x * right + move.y * forward;
            */
        }else{
            // Bouncing logic
            constexpr float PlayerSpeed = 2.0f;
            player.interpolation_time += PlayerSpeed * elapsed;
            auto new_pos = player.player_bounce_spline.at(player.interpolation_time);
            player.transform->position = new_pos;

            // Get back to walkmesh mode as we finish the interpolation
            if (player.interpolation_time >= 1.0){
                // Check if this is the second jump?
                if (player.bounce_stage == 1){
                    player.interpolation_time = 0.0;
                    player.bounce_stage = 2;
                    Sound::play(*bounce_sample);
                    set_bouncing_spline(player.bounce_destination,player.bounce_midpoint);
                    player.bounce_destination = glm::vec3{0.0f};
                    player.bounce_midpoint = glm::vec3{0.0f};

                }else{
                    if(player.bounce_stage != 2){
                        std::runtime_error("Wrong Bounce Stage");
                    }
                    player.interpolation_time = 0.0;
                    player.on_walkmesh = true;
                    player.at = walkmesh->nearest_walk_point(player.transform->position);
                    player.transform->position = walkmesh->to_world_point(player.at);
                    player.bounce_stage = 0;
                }




            }
        }
        
    }
    
    auto bbox = scene->collider_name_map[player.name];
    bbox->update_BBox(player.transform);
    
    //reset button press counters:
    left.downs = 0;
    right.downs = 0;
    up.downs = 0;
    down.downs = 0;
	lrun.downs = 0;
	rrun.downs = 0;
}

int lastWidth = -1;
int lastHeight = -1;

void PlayMode::resize_depth_tex() {
    int lw = lastWidth;
    int lh = lastHeight;
    
    SDL_GL_GetDrawableSize(window, &lastWidth, &lastHeight);
    if (lastWidth == lw && lastHeight == lh)
        return;
    
    glm::uvec2 window_size = glm::uvec2(lastWidth, lastHeight);
    
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei)window_size.x, (GLsizei)window_size.y, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindTexture(GL_TEXTURE_2D, shadow_depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei)(window_size.x * 4.0), (GLsizei)(window_size.y * 4.0), 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PlayMode::gen_dot_texture() {
    glGenTextures(1, &dot_tex);
    
    glm::uvec2 size;
    std::vector<glm::u8vec4> data;
    load_png(data_path("dot.png"), &size, &data, LowerLeftOrigin);
    
    glBindTexture(GL_TEXTURE_2D, dot_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    GL_ERRORS();
}

void PlayMode::gen_R_texture() {
    glGenTextures(1, &R_tex);
    
    glm::uvec2 size;
    std::vector<glm::u8vec4> data;
    load_png(data_path("R.png"), &size, &data, LowerLeftOrigin);
    
    glBindTexture(GL_TEXTURE_2D, R_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    GL_ERRORS();
}

// Code derived from https://15466.courses.cs.cmu.edu/lesson/framebuffers
void PlayMode::gen_framebuffers() {
    gen_dot_texture();
    
    glGenFramebuffers(1, &depth_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fb);
    glGenTextures(1, &depth_tex);
    
    glGenFramebuffers(1, &shadow_depth_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fb);
    glGenTextures(1, &shadow_depth_tex);
    
    resize_depth_tex();
    
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Draw an R sign as a hint to the player
void PlayMode::draw_keyboard_sign(glm::vec3 clip_space){

    static GLuint Rbuffer = 0;
    if (Rbuffer == 0) {
        glGenBuffers(1, &Rbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, Rbuffer);
        //actually nothing to do right now just wanted to bind it for illustrative purposes
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    struct Vert {
        Vert(glm::vec3 const &position_, glm::vec2 const &tex_coord_) : position(position_), tex_coord(tex_coord_) { }
        glm::vec3 position;
        glm::vec2 tex_coord;
    };
    static_assert(sizeof(Vert) == 20, "Vert is packed");

    auto &program = texture_program;

    static GLuint R_vao = 0;
    if (R_vao == 0) {
        //based on PPU466.cpp

        glGenVertexArrays(1, &R_vao);
        glBindVertexArray(R_vao);

        glBindBuffer(GL_ARRAY_BUFFER, Rbuffer);

        glVertexAttribPointer(
            program->Position_vec4, //attribute
            3, //size
            GL_FLOAT, //type
            GL_FALSE, //normalized
            sizeof(Vert), //stride
            (GLbyte *)0 + offsetof(Vert, position) //offset
        );
        glEnableVertexAttribArray(program->Position_vec4);

        glVertexAttribPointer(
            program->TexCoord_vec2, //attribute
            2, //size
            GL_FLOAT, //type
            GL_FALSE, //normalized
            sizeof(Vert), //stride
            (GLbyte *)0 + offsetof(Vert, tex_coord) //offset
        );
        glEnableVertexAttribArray(program->TexCoord_vec2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }


    //actually draw some textured quads!
    std::vector< Vert > attribs;

    clip_space.y += 0.1f;

    attribs.emplace_back(glm::vec3(clip_space.x - 0.03f, clip_space.y - 0.03f, 0.0f), glm::vec2(0.0f, 0.0f));
    attribs.emplace_back(glm::vec3(clip_space.x - 0.03f, clip_space.y + 0.03f, 0.0f), glm::vec2(0.0f, 1.0f));
    attribs.emplace_back(glm::vec3(clip_space.x + 0.03f, clip_space.y - 0.03f, 0.0f), glm::vec2(1.0f, 0.0f));
    attribs.emplace_back(glm::vec3(clip_space.x + 0.03f, clip_space.y + 0.03f, 0.0f), glm::vec2(1.0f, 1.0f));

    glBindBuffer(GL_ARRAY_BUFFER, Rbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vert) * attribs.size(), attribs.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //as per Scene::draw -
    glUseProgram(program->program);
    glUniformMatrix4fv(program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    glBindTexture(GL_TEXTURE_2D, R_tex);


    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(R_vao);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei) attribs.size());

    glBindVertexArray(0);


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    GL_ERRORS();

    glEnable(GL_DEPTH_TEST);

}





void PlayMode::draw_black_screen(){
    
    static GLuint tex = 0;
    if (tex == 0) {
        glGenTextures(1, &tex);

        glBindTexture(GL_TEXTURE_2D, tex);
        std::vector< glm::u8vec4 > tex_data{
            glm::u8vec4(0x00, 0x00, 0x00, 0xff), glm::u8vec4(0x00, 0x00, 0x00, 0xff),
            glm::u8vec4(0x00, 0x00, 0x00, 0xff), glm::u8vec4(0x00, 0x00, 0x00, 0xff)
        };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static GLuint buffer = 0;
    if (buffer == 0) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        //actually nothing to do right now just wanted to bind it for illustrative purposes
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    struct Vert {
        Vert(glm::vec3 const &position_, glm::vec2 const &tex_coord_) : position(position_), tex_coord(tex_coord_) { }
        glm::vec3 position;
        glm::vec2 tex_coord;
    };
    static_assert(sizeof(Vert) == 20, "Vert is packed");

    auto &program = texture_program;

    static GLuint vao = 0;
    if (vao == 0) {
        //based on PPU466.cpp

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);

        glVertexAttribPointer(
            program->Position_vec4, //attribute
            3, //size
            GL_FLOAT, //type
            GL_FALSE, //normalized
            sizeof(Vert), //stride
            (GLbyte *)0 + offsetof(Vert, position) //offset
        );
        glEnableVertexAttribArray(program->Position_vec4);

        glVertexAttribPointer(
            program->TexCoord_vec2, //attribute
            2, //size
            GL_FLOAT, //type
            GL_FALSE, //normalized
            sizeof(Vert), //stride
            (GLbyte *)0 + offsetof(Vert, tex_coord) //offset
        );
        glEnableVertexAttribArray(program->TexCoord_vec2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }


    //actually draw some textured quads!
    std::vector< Vert > attribs;

    attribs.emplace_back(glm::vec3(-1.0f, -1.0f, 0.9f), glm::vec2(0.0f, 0.0f));
    attribs.emplace_back(glm::vec3(-1.0f,  1.0f, 0.9f), glm::vec2(0.0f, 2.0f));
    attribs.emplace_back(glm::vec3( 1.0f, -1.0f, 0.9f), glm::vec2(1.0f, 0.0f));
    attribs.emplace_back(glm::vec3( 1.0f,  1.0f, 0.9f), glm::vec2(1.0f, 2.0f));

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vert) * attribs.size(), attribs.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //as per Scene::draw -
    glUseProgram(program->program);
    glUniformMatrix4fv(program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    glBindTexture(GL_TEXTURE_2D, tex);


    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei) attribs.size());

    glBindVertexArray(0);


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    GL_ERRORS();
    

    end_timepoint = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_timepoint - start_timepoint);

    if (duration.count() > 3){
        is_changing_scene = false;
        text_display.deactivate();
        text_display.remove_all_text();
    }

    glEnable(GL_DEPTH_TEST);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
    if (is_changing_scene){

        draw_black_screen();
        Draw::handle_all();
        return;
    }

    resize_depth_tex();
    //update camera aspect ratio for drawable:
    player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);
    
    //set up light type and position for lit_color_texture_program:
    // TODO: consider using the Light(s) in the scene to do this
    glUseProgram(lit_color_texture_program->program);
    glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
    glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1,
                 glm::value_ptr(glm::normalize(glm::vec3(0.5f, 1.0f, -1.0f))));
    glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(0.85f, 0.85f, 0.85f)));
    glUniform3fv(lit_color_texture_program->AMBIENT_LIGHT_ENERGY_vec3, 1,
                 glm::value_ptr(glm::vec3(0.25f, 0.25f, 0.25f)));
    
    int w, h;
    int wn, hn;
    SDL_GL_GetDrawableSize(window, &w, &h);
    SDL_GetWindowSize(window, &wn, &hn);
    glm::vec4 window_size = glm::vec4(w, h, wn, hn);
    glUniform4fv(lit_color_texture_program->WINDOW_DIMENSIONS, 1, glm::value_ptr(window_size));
    glUseProgram(0);
    
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
    glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.
    
    
    // Draw the depth framebuffer for edge detection
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fb);
    glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    scene->draw(*player.camera, false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    scene->draw(*player.camera, true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Draw the depth framebuffer for edge detection
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fb);
    glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    scene->draw(*player.camera, false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    scene->draw(*player.camera, true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dot_tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_tex);
    glActiveTexture(GL_TEXTURE0);

    glViewport(0, 0, (GLsizei)(drawable_size.x * 4.0), (GLsizei)(drawable_size.y * 4.0));
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fb);
    glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    scene->draw_shadow(*player.camera, false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    scene->draw_shadow(*player.camera, true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glViewport(0, 0, drawable_size.x, drawable_size.y);
    // Draw the world
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    scene->draw(*player.camera, false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    scene->draw(*player.camera, true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // {
    //     DrawLines lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
    //     for(auto r : rays){
    //         lines.draw(r.first,r.second);
    //     }
    // }

    {
        // Draw a sign

        // Currently it can not handle the situation when the sign is occluded by some other objects between it and the camera
        if(animated == NO){
            std::string name;
            glm::vec3 pos;
            std::tie(name,pos) = find_closest_sign();

            glm::mat4 world_to_clip = player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local());

            if (!name.empty()){
                
                glm::vec4 clip_space = world_to_clip * glm::vec4{pos,1.0};
                
                glm::vec3 clip_space_3d = glm::vec3{clip_space.x / clip_space.w,clip_space.y/clip_space.w,clip_space.z/clip_space.w};

                draw_keyboard_sign(clip_space_3d);
            }
        }

    }

    

    if(animated == NO)
    // Draw a crosshair at the center of the screen
    {
        glDisable(GL_DEPTH_TEST);
//        glm::vec2 center{0.0f,0.0f};
        float aspect = float(drawable_size.x) / float(drawable_size.y);
        DrawLines lines(glm::mat4(
                1.0f / aspect, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        ));
        
        glm::vec2 offset(0.05f, 0.05f);
        
        glm::vec3 pv_0 = {0.0f, 0.0f + offset[1], 0.0f};
        glm::vec3 pv_1 = {0.0f, 0.0f - offset[1], 0.0f};
        
        glm::vec3 ph_0 = {0.0f + offset[0], 0.0f, 0.0f};
        glm::vec3 ph_1 = {0.0f - offset[1], 0.0f, 0.0f};
        
        lines.draw(pv_0, pv_1);
        lines.draw(ph_0, ph_1);
        glEnable(GL_DEPTH_TEST);
    }

    
    
    //draw_image(image_vao, shadow_depth_tex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.4f, 0.4f, 0.2f, 0.2f);
    
    Draw::handle_all();
    GL_ERRORS();
}


// TODO exclude player collider?
void PlayMode::update_wireframe(const std::shared_ptr<Scene::Collider> &c) {
    if (!player.has_paint_ability) {
        text_display.add_text(std::vector<std::string>{"A magic aura lingers around this object,"});
        text_display.add_text(std::vector<std::string>{"but you can't seem to figure out the right spell."});
        text_display.add_text(std::vector<std::string>{"Maybe try looking around for some clues?"});
        return;
    }
    
    
    if (c->name.find("wire") == std::string::npos) {
        return;
    }
    
    {
        bool found = false;
        for (const auto &it: scene->wireframe_objects) {
            if (it->name == c->name) {
                found = true;
            }
        }
        if (!found)
            return;
    }
    
    
    if (!player.has_paint_ability) {
        if (c->name.find("Paintbrush") == std::string::npos) {
            return;
        }
    }
    
    bool is_current_wireframe = scene->drawble_name_map[c->name]->wireframe_info.draw_frame;
    auto d = scene->drawble_name_map[c->name];


    text_display.add_text(std::vector<std::string>{"You use the magic paintbrush's power on the object"});
    Sound::play(*cast_sample);
    
    if (is_current_wireframe) {
        scene->current_wireframe_objects_map.erase(c->name);
        if (scene->wf_obj_block_map.count(c->name)) {
            scene->colliders.push_back(c);
        } else if (scene->wf_obj_pass_map.count(c->name)) {
            scene->colliders.remove(c);
        } else {
            throw std::runtime_error("Run wireframe state");
        }
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            scene->wireframe_objects.remove(c);
            scene->wf_obj_block_map.erase(c->name);
            scene->wf_obj_pass_map.erase(c->name);
        }
        d->wireframe_info.draw_frame = false;


        // If this is the compass, trigger scenc change logic
        if(c->name.find("frontroom")!=std::string::npos){
            is_changing_scene = true;
            start_timepoint = std::chrono::system_clock::now();
            //change to foodworld?
            scene = scene_map[FOODSCENE];
            bgm->stop(1.0);
            text_display.remove_all_text();
 
            std::vector<std::string> tmpstr{
            "You paint in the compass, allowing you to",
            "take off and travel to another world"
            };
            text_display.add_text(tmpstr);
            if(!text_display.is_activated())
                text_display.activate();
            walkmesh = &foodworld_walkmeshes->lookup("WalkMesh");
            initialize_player();
            bgm = Sound::loop(*fire_sample);
        }


    } else {
        // remove bounding box
        if (scene->wf_obj_block_map.count(c->name)) {
            scene->colliders.remove(c);
        } else if (scene->wf_obj_pass_map.count(c->name)) {
            scene->colliders.push_back(c);
        }
        scene->current_wireframe_objects_map[c->name] = c;
        
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            scene->wireframe_objects.remove(c);
            scene->wf_obj_block_map.erase(c->name);
            scene->wf_obj_pass_map.erase(c->name);
            scene->current_wireframe_objects_map.erase(c->name);
        }
        d->wireframe_info.draw_frame = true;
    }
    
    if (c->name.find("Paintbrush") != std::string::npos) {
        player.has_paint_ability = true;
    }


}

void PlayMode::update_wireframe() {
    std::string name_to_real, name_to_wireframe;
    std::shared_ptr<Scene::Collider> collider_to_real = nullptr;  // Add back to fully draw
    std::shared_ptr<Scene::Collider> collider_to_wireframe = nullptr; // draw wireframe
    
    // Test the frame thing?
    
    auto c = scene->collider_name_map[player.name];
    
    if (player.has_paint_ability) {
        // remove real object, only draw wireframe
        for (const auto &collider: scene->wireframe_objects) {
            if (collider->name == player.name) {
                continue;
            }
            auto dist = c->min_distance(collider);
            if (dist < 0.5) {
                std::string name = collider->name;
                // If this is already a wireframe
                if (!scene->current_wireframe_objects_map.count(name)) {
                    collider_to_wireframe = collider;
                    name_to_wireframe = name;
                    break;
                    
                }
            }
        }
        // turn wireframe object real
        if (collider_to_wireframe == nullptr) {
            // Add it back
            for (const auto &it: scene->current_wireframe_objects_map) {
                const std::string &name = it.first;
                auto collider = it.second;
                auto dist = c->min_distance(collider);
                if (dist < 0.5 && !c->intersect(collider)) {
                    collider_to_real = collider;
                    name_to_real = name;
                    break;
                }
            }
            
        }
    } else { // Paintbrush case // This is ugly code but it works..
        for (const auto &collider: scene->wireframe_objects) {
            if (collider->name == player.name || collider->name.find("Paintbrush") == std::string::npos) {
                continue;
            }
            auto dist = c->min_distance(collider);
            if (dist < 0.5) {
                std::string name = collider->name;
                // If this is already a wireframe
                if (!scene->current_wireframe_objects_map.count(name)) {
                    collider_to_wireframe = collider;
                    name_to_wireframe = name;
                    player.has_paint_ability = true;
                    break;
                    
                }
            }
        }
        if (collider_to_wireframe == nullptr) {
            // Add it back
            for (const auto &it: scene->current_wireframe_objects_map) {
                const std::string &name = it.first;
                if (name.find("Paintbrush") == std::string::npos) {
                    continue;
                }
                
                auto collider = it.second;
                auto dist = c->min_distance(collider);
                if (dist < 0.5 && !c->intersect(collider)) {
                    collider_to_real = collider;
                    name_to_real = name;
                    player.has_paint_ability = true;
                    break;
                }
            }
            
        }
    }
    
    
    if (collider_to_real) {
        // Add back bounding box
        if (scene->wf_obj_block_map.count(name_to_real)) {
            scene->colliders.push_back(collider_to_real);
        }
            // remove virtual bounding box
        else if (scene->wf_obj_pass_map.count(name_to_real)) {
            scene->colliders.remove(collider_to_real);
        }
        
        scene->current_wireframe_objects_map.erase(name_to_real);
        auto d = scene->drawble_name_map[name_to_real];
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            scene->wireframe_objects.remove(collider_to_real);
            scene->wf_obj_block_map.erase(name_to_real);
            scene->wf_obj_pass_map.erase(name_to_real);
        }
        d->wireframe_info.draw_frame = false;
    }
    
    if (collider_to_wireframe) {
        // remove bounding box
        if (scene->wf_obj_block_map.count(name_to_wireframe)) {
            scene->colliders.remove(collider_to_wireframe);
        } else if (scene->wf_obj_pass_map.count(name_to_wireframe)) {
            scene->colliders.push_back(collider_to_wireframe);
        }
        
        
        scene->current_wireframe_objects_map[name_to_wireframe] = collider_to_wireframe;
        auto d = scene->drawble_name_map[name_to_wireframe];
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            scene->wireframe_objects.remove(collider_to_wireframe);
            scene->wf_obj_block_map.erase(name_to_wireframe);
            scene->wf_obj_pass_map.erase(name_to_wireframe);
            scene->current_wireframe_objects_map.erase(name_to_wireframe);
        }
        d->wireframe_info.draw_frame = true;
    }
}





// Item to unlock must be a collider
void PlayMode::unlock(const std::string &prefix) {
    
    auto c = scene->collider_name_map[player.name];
    
    std::shared_ptr<Scene::Collider> collider_to_remove = nullptr;
    std::string name_to_remove;
    
    for (const auto &collider: scene->colliders) {
        if (collider->name.find(prefix) != std::string::npos) {
            auto dist = c->min_distance(collider);
            if (dist < 2.0) {
                collider_to_remove = collider;
                name_to_remove = collider->name;
                break;
            }
        } else {
            continue;
        }
    }
    // Remove it from drawables and collider datastructure
    auto d = scene->drawble_name_map[name_to_remove];
    scene->drawables.remove(d);
    scene->drawble_name_map.erase(name_to_remove);
    scene->colliders.remove(collider_to_remove);
    scene->collider_name_map.erase(name_to_remove);
}


std::pair<std::shared_ptr<Scene::Collider>, float>
PlayMode::mouse_text_check(const std::string &prefix, bool use_crosshair) {
    float ux, uy;
    
    if (!use_crosshair) {
        if (SDL_GetRelativeMouseMode() != SDL_FALSE)
            return std::make_pair(nullptr, 0.0f);
        
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        y = 720 - y;
        
        ux = ((float) x - 640.0f) / 640.0f;
        uy = ((float) y - 360.0f) / 360.0f;
    } else {
        ux = 0.0;
        uy = 0.0;
    }
    
    
    // nearest plane. In the basecode, nearest plane will be mapped to -1.0 and far plane(inifinity) will be mapped to 1.0
    glm::vec4 nearpoint{ux, uy, -1.0, 1.0};
    
    glm::mat4 world_to_clip =
            player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local());
    
    glm::mat4 inv_world_to_clip = glm::inverse(world_to_clip);
    
    glm::vec4 near_result = inv_world_to_clip * nearpoint;
    near_result /= near_result.w;
    
    // Camera world position should be obtained like this
    auto camera_to_world = player.camera->transform->make_local_to_world();
    glm::vec3 camera_world_location = {camera_to_world[3][0], camera_to_world[3][1], camera_to_world[3][2]};
    
    
    Ray dir = Ray{camera_world_location,
                  glm::vec3{near_result.x, near_result.y, near_result.z} - camera_world_location};
    
    std::shared_ptr<Scene::Collider> intersected_collider = nullptr;
    
    for (const auto &it: scene->textcollider_name_map) {
        auto c = it.second;
        if (c->name.find(prefix) != std::string::npos) {
            bool intersected;
            float t;
            std::tie(intersected, t) = c->ray_intersect(dir);
            if (intersected) {
                if (t < dir.t) {
                    dir.t = t;
                    intersected_collider = c;
                }
            }
        }
    }
    
    
    float distance = glm::length(dir.d * dir.t);
    
    
    return std::make_pair(intersected_collider, distance);
}

std::pair<std::shared_ptr<Scene::Collider>, float>
PlayMode::mouse_collider_check(const std::string &prefix, bool use_crosshair) {
    float ux, uy;
    
    if (!use_crosshair) {
        if (SDL_GetRelativeMouseMode() != SDL_FALSE)
            return std::make_pair(nullptr, 0.0f);
        
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        y = 720 - y;
        
        ux = ((float) x - 640.0f) / 640.0f;
        uy = ((float) y - 360.0f) / 360.0f;
    } else {
        ux = 0.0;
        uy = 0.0;
    }
    
    
    
    
    // nearest plane. In the basecode, nearest plane will be mapped to -1.0 and far plane(inifinity) will be mapped to 1.0
    glm::vec4 nearpoint{ux, uy, -1.0, 1.0};
    
    glm::mat4 world_to_clip =
            player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local());
    
    glm::mat4 inv_world_to_clip = glm::inverse(world_to_clip);
    
    glm::vec4 near_result = inv_world_to_clip * nearpoint;
    near_result /= near_result.w;
    
    // Camera world position should be obtained like this
    auto camera_to_world = player.camera->transform->make_local_to_world();
    glm::vec3 camera_world_location = {camera_to_world[3][0], camera_to_world[3][1], camera_to_world[3][2]};
    
    
    Ray dir = Ray{camera_world_location,
                  glm::vec3{near_result.x, near_result.y, near_result.z} - camera_world_location};
    
    std::shared_ptr<Scene::Collider> intersected_collider = nullptr;
    

    if(prefix.find("terminal")!=std::string::npos){
        for (const auto &it: scene->terminal_name_map) {
            auto c = it.second;
            if (c->name.find(prefix) != std::string::npos || c->name.find("Paintbrush") != std::string::npos) {
                bool intersected;
                float t;
                std::tie(intersected, t) = c->ray_intersect(dir);
                if (intersected) {
                    if (t < dir.t) {
                        dir.t = t;
                        intersected_collider = c;
                    }
                }
            }
            
            
        }
    }else{
        for (const auto &it: scene->collider_name_map) {
            auto c = it.second;
            if (c->name.find(prefix) != std::string::npos || c->name.find("Paintbrush") != std::string::npos) {
                bool intersected;
                float t;
                std::tie(intersected, t) = c->ray_intersect(dir);
                if (intersected) {
                    if (t < dir.t) {
                        dir.t = t;
                        intersected_collider = c;
                    }
                }
            }
            
            
        }
    }

    
    
    float distance = glm::length(dir.d * dir.t);
    return std::make_pair(intersected_collider, distance);
}




std::pair<std::shared_ptr<Scene::Collider>, float>
PlayMode::mouse_bread_check(const std::string &prefix, bool use_crosshair) {
    float ux, uy;
    
    if (!use_crosshair) {
        if (SDL_GetRelativeMouseMode() != SDL_FALSE)
            return std::make_pair(nullptr, 0.0f);
        
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        y = 720 - y;
        
        ux = ((float) x - 640.0f) / 640.0f;
        uy = ((float) y - 360.0f) / 360.0f;
    } else {
        ux = 0.0;
        uy = 0.0;
    }
    
    
    
    
    // nearest plane. In the basecode, nearest plane will be mapped to -1.0 and far plane(inifinity) will be mapped to 1.0
    glm::vec4 nearpoint{ux, uy, -1.0, 1.0};
    
    glm::mat4 world_to_clip =
            player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local());
    
    glm::mat4 inv_world_to_clip = glm::inverse(world_to_clip);
    
    glm::vec4 near_result = inv_world_to_clip * nearpoint;
    near_result /= near_result.w;
    
    // Camera world position should be obtained like this
    auto camera_to_world = player.camera->transform->make_local_to_world();
    glm::vec3 camera_world_location = {camera_to_world[3][0], camera_to_world[3][1], camera_to_world[3][2]};
    
    
    Ray dir = Ray{camera_world_location,
                  glm::vec3{near_result.x, near_result.y, near_result.z} - camera_world_location};
    
    std::shared_ptr<Scene::Collider> intersected_collider = nullptr;
    
    for (const auto &it: scene->breadcollider_name_map) {
        auto c = it.second;
        if (c->name.find(prefix) != std::string::npos || c->name.find("Paintbrush") != std::string::npos) {
            bool intersected;
            float t;
            std::tie(intersected, t) = c->ray_intersect(dir);
            if (intersected) {
                if (t < dir.t) {
                    dir.t = t;
                    intersected_collider = c;
                }
            }
        }
        
        
    }
    
    float distance = glm::length(dir.d * dir.t);
    return std::make_pair(intersected_collider, distance);
}


ColliderType PlayMode::check_collider_type(std::shared_ptr<Scene::Collider> c) {
    if (!c) {
        std::runtime_error("NULL pointer");
        return UNKNOWN;
    } else {
        const std::string &name = c->name;
        
        if (name.find("col_wire") != std::string::npos) {
            return WIREFRAME;
        } else if (name.find("col_unlock") != std::string::npos) {
            return DOOR;
        } else if (name.find("col_food") != std::string::npos) {
            return FOOD;
        } else {
            std::runtime_error("Unkonwn collider type");
            return UNKNOWN;
        }
        
    }
    
}




void PlayMode::initialize_scene(Load<Scene> scene_to_copy, Load<MeshBuffer> meshbuffer_param, scene_type scene_param_type){
    
    std::shared_ptr<Scene> new_scene = std::make_shared<Scene>(*scene_to_copy);


    //create a player transform:
    //new_scene->transforms.emplace_back();

    Scene::Transform *player_transform = nullptr;
    
    for (auto &t: new_scene->transforms) {
        if (t.name == player.name) {
            player_transform = &t;
            break;
        }
    }

    if(player_transform == nullptr){
        std::runtime_error("Player transform doesn't exist in scene file");
    }

    Mesh const &mesh = wizard_meshes->lookup("wizard");
    new_scene->drawables.emplace_back(std::make_shared<Scene::Drawable>(player_transform));
    std::shared_ptr<Scene::Drawable> wizard_drawable = new_scene->drawables.back();
    
    wizard_drawable->pipeline = rocket_color_texture_program_pipeline;
    
    wizard_drawable->pipeline.vao = wizard_meshes_for_lit_color_texture_program;
    wizard_drawable->pipeline.type = mesh.type;
    wizard_drawable->pipeline.start = mesh.start;
    wizard_drawable->pipeline.count = mesh.count;
    wizard_drawable->specular_info.shininess = 10.0f;
    wizard_drawable->specular_info.specular_brightness = glm::vec3(1.0f, 0.9f, 0.7f);
    wizard_drawable->scene_info.type = scene_param_type;
    
    new_scene->initialize_scene_metadata();
    new_scene->initialize_collider("col_", meshbuffer_param);
    new_scene->initialize_wireframe_objects("col_wire");
    new_scene->initialize_text_collider("text_", meshbuffer_param);
    new_scene->initialize_bread("bread_",meshbuffer_param);

    scene_map[scene_param_type] = new_scene;
}


void PlayMode::initialize_player(){

    //player.transform = &scene->transforms.back();
    //look up player transform
    for (auto &t: scene->transforms) {
        if (t.name == player.name) {
            player.transform = &t;
            break;
        }
    }
    
    //create a player camera attached to a child of the player transform:
    scene->transforms.emplace_back();
    scene->cameras.emplace_back(&scene->transforms.back());
    player.camera = &scene->cameras.back();
    player.camera->fovy = glm::radians(60.0f);
    player.camera->near = 0.01f;
    player.camera->transform->parent = player.transform;
    player.camera->transform->name = "player_c";
    scene->cams["player_c"] = player.camera;
    
    player.camera->transform->position = player.defaultCameraPosition;
    player.camera->transform->rotation = player.defaultCameraRotation;
    //glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    //start player walking at nearest walk point:
    player.at = walkmesh->nearest_walk_point(player.transform->position);
    player.on_walkmesh = true;


    player.add_component<TerminalCommandHandler>([this](Command command) {
        std::string pb_object_name = "col_wire_off_block_Paintbrush";
        switch (command) {
            case Command::OpenSesame:
                //unlock("unlock_");
                //unlock("unlock_");
                player.has_unlock_ability = true;
                std::cout << "command was open sesame!\n";
                break;
            case Command::Mirage:
                //activate paintbrush
                if (!player.has_paint_ability) {
                    auto pb = scene->collider_name_map[pb_object_name];
                    
                    float distance = pb->min_distance(scene->collider_name_map[player.name]);
                    
                    if (distance < 10) {
                        auto d = scene->drawble_name_map[pb_object_name];
                        assert(d->wireframe_info.draw_frame);
                        d->wireframe_info.draw_frame = false;
                        player.has_paint_ability = true;
                        scene->colliders.push_back(pb);
                        scene->current_wireframe_objects_map.erase(pb_object_name);
                        if (d->wireframe_info.one_time_change) {
                            scene->wireframe_objects.remove(pb);
                            scene->wf_obj_block_map.erase(pb_object_name);
                            scene->wf_obj_pass_map.erase(pb_object_name);
                            
                        }
                    }
                }
                
                //update_wireframe();
                std::cout << "command was mirage!\n";
                break;
            case Command::Cook:
                cook();
                break;
        }
    });
    
    player.add_component<TerminalDeactivateHandler>([this]() {
        player.add_component<EventHandler>([this](SDL_Event const &evt, glm::uvec2 const &window_size) {
            if (evt.type == SDL_KEYDOWN) {
                if (evt.key.keysym.sym == SDLK_ESCAPE) {
                    // no longer disables relative mouse, that is handled in main
                    if (text_display.is_activated()) {
                        text_display.deactivate();
                        text_display.remove_all_text();
                    }
                    esc.downs += 1;
                    esc.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_a) {
                    left.downs += 1;
                    left.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_d) {
                    right.downs += 1;
                    right.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_w) {
                    up.downs += 1;
                    up.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_s) {
                    down.downs += 1;
                    down.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_LSHIFT) {
                    lrun.downs += 1;
                    lrun.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_RSHIFT) {
					rrun.downs += 1;
					rrun.pressed = true;
					return true;
				} else if (evt.key.keysym.sym == SDLK_e) {
                    std::shared_ptr<Scene::Collider> c = nullptr;
                    float distance = 0.0;
                    std::tie(c, distance) = mouse_collider_check("col_terminal", true);
                    if(c){
                        auto player_c = scene->collider_name_map["Player"];
                        auto location_player = (player_c->min + player_c->max) / 2.0f;

                        if(c->point_intersect(location_player)){
                            terminal.activate();
                            left.pressed = false;
                            right.pressed = false;
                            up.pressed = false;
                            down.pressed = false;
                            read.pressed = false;
                            player.remove_component<EventHandler>();
                            return true;
                        }else{
                            return true;
                        }

                    }else{
                        return true;
                    }



                } else if (evt.key.keysym.sym == SDLK_r) {
                    read.downs += 1;
                    read.pressed = true;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_SPACE) {
                    std::shared_ptr<Scene::Collider> c = nullptr;
                    float distance = 0.0;
                    
                    std::tie(c, distance) = mouse_collider_check("col_", true);
                    if (c) {
                        auto type = check_collider_type(c);
                        switch (type) {
                            case WIREFRAME: {
//                                if(!player.has_paint_ability){
//                                    text_display.add_text(std::vector<std::string>{"A magic aura lingers around this object,"});
//                                    text_display.add_text(std::vector<std::string>{"but you can't seem to figure out the right spell."});
//                                    text_display.add_text(std::vector<std::string>{"Maybe try looking around for some clues?"});
//                                    text_display.activate();
//                                }else
                                {
                                    auto player_collider = scene->collider_name_map[player.name];
                                    if (distance < 10.0f) {
                                        // Do not update if player intersects the object
                                        if (!player_collider->intersect(c)){
                                            update_wireframe(c);
                                            //text_display.add_text(std::vector<std::string>{"You cast wireframe magic to the object"});
                                        }else{
                                            text_display.add_text(std::vector<std::string>{"You are too close to the object. Casting magic at such distance will hurt you!"});
                                        }
                                        
                                    }else{
                                        text_display.add_text(std::vector<std::string>{"You are too far away from the object"});
                                    }

                                    text_display.activate();

                                }
                                break;
                               
                            }
                            
                            case DOOR: {
                                if (player.has_unlock_ability) {
                                    auto player_collider = scene->collider_name_map[player.name];
                                    if (distance < 10.0f) {
                                        // Remove it from drawables and collider datastructure
                                        auto d = scene->drawble_name_map[c->name];
                                        scene->drawables.remove(d);
                                        scene->drawble_name_map.erase(c->name);
                                        scene->colliders.remove(c);
                                        scene->collider_name_map.erase(c->name);
                                        text_display.add_text(std::vector<std::string>{"You unlocked the door!"});
                                        text_display.activate();

                                        Sound::play(*door_open_sample);
                                        {
                                            //debug lines for pause menu
                                            // Mode::set_state(END);
                                            // text_display.deactivate();
                                        }


                                        break;
                                    }
                                }


                                
                            }
                            case FOOD:
                                break;
                            default:
                                break;
                        }
                        
                        
                    } else{
                        std::tie(c,distance) = mouse_bread_check("bread_",true);
                        if(c){
                            if(player.has_bounce_ability){
                                glm::vec3 location{0.0f, 0.0f, 0.0f};
                                if (endsWith(c->name,"_1")){
                                    location = (c->min + c->max) / 2.0f;
                                }else{
                                    std::runtime_error("Unknown bread collider");
                                }

                                text_display.add_text(std::vector<std::string>{"You are jumping to a bread!"});
                                text_display.activate();

                                get_off_walkmesh();
                                set_bouncing_spline(location);
                                Sound::play(*bounce_sample);
                                player.bounce_stage = 1;
                                player.bounce_destination = scene->bread_bouncelocation_map[c].second;
                                player.bounce_midpoint = scene->bread_bouncelocation_map[c].first;
                                
                            }
                        }
                    }
                    
                    //update_wireframe();
                    return true;
                }

            } else if (evt.type == SDL_KEYUP) {
                if (evt.key.keysym.sym == SDLK_a) {
                    left.pressed = false;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_d) {
                    right.pressed = false;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_w) {
                    up.pressed = false;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_s) {
                    down.pressed = false;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_LSHIFT) {
                    lrun.pressed = false;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_RSHIFT) {
					rrun.pressed = false;
					return true;
				} else if (evt.key.keysym.sym == SDLK_r) {
                    read.pressed = false;
                    return true;
                } else if (evt.key.keysym.sym == SDLK_ESCAPE) {
                    esc.pressed = false;
                    return true;
                }
            } else if (evt.type == SDL_MOUSEBUTTONDOWN) {
                if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    return true;
                }
            } else if (animated == NO && evt.type == SDL_MOUSEMOTION) {
                // don't want camera to change when reading sign
                if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
                    glm::vec2 motion = glm::vec2(
                            evt.motion.xrel / float(window_size.y),
                            -evt.motion.yrel / float(window_size.y)
                    );
                    glm::vec3 upDir = walkmesh->to_world_smooth_normal(player.at);
                    player.transform->rotation =
                            glm::angleAxis(-motion.x * player.camera->fovy, upDir) * player.transform->rotation;
                    
                    float pitch = glm::pitch(player.camera->transform->rotation);
                    pitch += motion.y * player.camera->fovy;
                    //camera looks down -z (basically at the player's feet) when pitch is at zero.
                    pitch = std::min(pitch, 0.80f * glm::pi<glm::float32>());
                    pitch = std::max(pitch, 0.05f * glm::pi<glm::float32>());
                    player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

                    float extra = 0.0f;
                    if (pitch >= 0.50f * glm::pi<glm::float32>())
                        extra = (pitch - 0.50f * glm::pi<glm::float32>()) / (0.30f * glm::pi<glm::float32>());
                    
                    const glm::float32 DIST_TO_PLAYER = glm::length(player.defaultCameraPosition);
                    player.camera->transform->position =
                            -player.camera->transform->rotation * glm::vec3(-1.0f, 2.0f, DIST_TO_PLAYER * (1.0f - extra));
                    
                    
                    return true;
                }
            }
            
            return false;
        });
    });
}



void PlayMode::get_off_walkmesh(){
    if (player.on_walkmesh){
        player.on_walkmesh = false;
    }else{
        std::runtime_error("Trying to get off the walkmesh when it's already off");
    }
}


void PlayMode::set_bouncing_spline(glm::vec3 destination, glm::vec3 midpoint){

    std::cerr << "This is a debug test for spline interpolation of player location" << std::endl;

    //Hard code / Debug code for now.
    
    // Go to a location 
    auto player_to_world = player.transform->make_local_to_world();
    glm::vec3 player_world_location = {player_to_world[3][0], player_to_world[3][1], player_to_world[3][2]};

    glm::vec3 bounce_location = destination;

    if (midpoint.x == midpoint.y && midpoint.y == midpoint.z && midpoint.x == 0){
        // Add a control point to the spline?
        midpoint.z  = destination.z + 2;
        midpoint.x  = (destination.x - player_world_location.x) / 3 + player_world_location.x;
        midpoint.y  = (destination.y - player_world_location.y) / 3 + player_world_location.y;
    }




    player.player_bounce_spline.clear();
    player.interpolation_time = 0.0f;

    player.player_bounce_spline.set(0.0f,player_world_location);
    player.player_bounce_spline.set(1.0f, bounce_location);
    
    // Add an extra control point
    player.player_bounce_spline.set(0.5f,midpoint);

}



std::pair<std::string,glm::vec3>  PlayMode::find_closest_sign(){
        //float distance = std::numeric_limits<float>::max();
        float distance = player.SIGHT_DISTANCE; // can see this far
        auto here = player.transform->position;
        //std::cout << "here " << here.x << " " << here.y << " " << here.z << std::endl;
        std::string selected;
        
        // find the closest text bearer
        for (const auto &[name, mesh]: textBearers) {
            if (!endsWith(name, "_m")) {
                continue;
            }
            auto transform = nameToTransform[name];
            // Is this correct???
            //auto there = transform->make_local_to_world() * glm::vec4(transform->position, 1.0);
            auto tmp = transform->make_local_to_world();
            auto there = glm::vec3{tmp[3][0],tmp[3][1],tmp[3][2]};
            //std::cout << "there " << there.x << " " << there.y << " " << there.z << std::endl;
            float newdistance = glm::distance(here, there);
            //std::cout << "distance: " << newdistance << " name: " << name << std::endl;
            if (newdistance < distance) {
                distance = newdistance;
                selected = name;
            }
        }
        if (!selected.empty()) {
            if (scene->collider_name_map.count(selected) == 0){
                //std::cout << "selected: " << selected << std::endl;
                assert(selected.back() == 'm');
                std::string selectedCamera = textBearerCams[selected];
                auto t = nameToTransform[selected];
                auto selectedToWorld = t->make_local_to_world();
                auto world_coord = glm::vec3{selectedToWorld[3][0],selectedToWorld[3][1],selectedToWorld[3][2]};
                //auto endposition = selectedToWorld * glm::vec4(destCamera->transform->position, 1.0);
                return std::make_pair(selected,world_coord);
            }else{
                auto c = scene->collider_name_map[selected];
                auto p = (c->min + c->max) / 2.0f;
                p.z = c->max.z;
                return std::make_pair(selected,p);
            }
        } else {
            //std::cout << "No readable sign in range" << std::endl;
            return std::make_pair(selected,glm::vec3{0.0f});
        }
    }

void PlayMode::cook() {
    for (auto &[name, _] : scene->drawble_name_map) {
        std::cout << "collider has name " << name << "\n";
    }
    const std::unordered_map<std::string, bool> ingredients {
            {"col_wire_off_pass_breadstick", true}
    };
    bool correct = true;
    for (auto &[name, required] : ingredients) {
        bool is_current_wireframe = scene->drawble_name_map[name]->wireframe_info.draw_frame;
        if (is_current_wireframe == required) {
            correct = false;
        }
    }
    if (correct) {
        terminal.text_display.add_text({"You made some delicious food. Now you can bounce on bread."});
        player.has_bounce_ability = true;
    } else {
        terminal.text_display.add_text({"You used the wrong ingredients... it tastes awful :("});
        // Reset those ingredients to wireframe
        for (auto &[name, required] : ingredients) {
            bool is_current_wireframe = scene->drawble_name_map[name]->wireframe_info.draw_frame;
            if (!is_current_wireframe) {
                update_wireframe(scene->collider_name_map[name]);
            }
        }
    }
}

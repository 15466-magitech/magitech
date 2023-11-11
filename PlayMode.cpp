#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "RocketColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "TextStorage.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Components/EventHandler.hpp"
#include "spline.h"


#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>

GLuint artworld_meshes_for_lit_color_texture_program = 0;
GLuint artworld_meshes_for_rocket_color_texture_program = 0;
GLuint textcube_meshes_for_lit_color_texture_program = 0;
GLuint wizard_meshes_for_lit_color_texture_program = 0;

// object name to transform
std::unordered_map<std::string, Scene::Transform *>nameToTransform;
// text bearer name to mesh
std::unordered_map<std::string, Mesh const *>textBearers;
// text bearer name to camera
std::unordered_map<std::string, std::string>textBearerCams;

// c++ still sucks
bool endsWith(const std::string& str, const std::string& suffix) {
  if (str.length() < suffix.length()) {
    return false;
  }
  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

Load<MeshBuffer> artworld_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("artworld.pnct"));
    artworld_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    artworld_meshes_for_rocket_color_texture_program = ret->make_vao_for_program(rocket_color_texture_program->program);

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
    wizard_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
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
                    //drawable->pipeline = lit_color_texture_program_pipeline;
                    //drawable->pipeline.vao = artworld_meshes_for_lit_color_texture_program;
                    drawable->pipeline = rocket_color_texture_program_pipeline;
                    drawable->pipeline.vao = artworld_meshes_for_rocket_color_texture_program;
                    drawable->specular_info.shininess = 10.0;
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
            });
});

WalkMesh const *walkmesh = nullptr;
Load<WalkMeshes> artworld_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
    auto *ret = new WalkMeshes(data_path("artworld.w"));
    walkmesh = &ret->lookup("WalkMesh");
    return ret;
});

PlayMode::PlayMode()
        : terminal(10, 30, glm::vec2(0.05f, 0.05f), glm::vec2(0.4f, 0.4f)),
          text_display(5, 75, glm::vec2(-0.50f, -0.50f), glm::vec2(1.0f, 0.2f)),
          scene(*artworld_scene) {
    // TODO: remove this test code
    std::cout << "Testing basic ECS mechanics..." << std::endl;
    {
        Entity a;
        a.add_component<EventHandler>([](const SDL_Event &evt, const glm::uvec2 &window_size) {
            return false;
        });
    }
    std::cout << "Success!" << std::endl;
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
    
    //create a player transform:
    scene.transforms.emplace_back();
    
    for (auto &t: scene.transforms) {
        if (t.name == player.name) {
            player.transform = &t;
        }
    }
    //player.transform = &scene.transforms.back();
    
    //create a player camera attached to a child of the player transform:
    scene.transforms.emplace_back();
    scene.cameras.emplace_back(&scene.transforms.back());
    player.camera = &scene.cameras.back();
    player.camera->fovy = glm::radians(60.0f);
    player.camera->near = 0.01f;
    player.camera->transform->parent = player.transform;
    player.camera->transform->name = "player_c";
    scene.cams["player_c"] = player.camera;
    
    //default view point behind player
    player.camera->transform->position = glm::vec3(-0.0f, -5.0f, 2.5f);
    
    //rotate camera to something pointing in way of player
    // arcsin 0.1 ~ 6 degrees
    player.camera->transform->rotation = glm::vec3(glm::radians(84.0f), glm::radians(0.0f), glm::radians(0.0f));
    //glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    //start player walking at nearest walk point:
    player.at = walkmesh->nearest_walk_point(player.transform->position);
    
    Scene::Transform *transform = player.transform;
    Mesh const &mesh = wizard_meshes->lookup("wizard");
    scene.drawables.emplace_back(std::make_shared<Scene::Drawable>(transform));
    std::shared_ptr<Scene::Drawable> wizard_drawable = scene.drawables.back();
    
    wizard_drawable->pipeline = lit_color_texture_program_pipeline;
    
    wizard_drawable->pipeline.vao = wizard_meshes_for_lit_color_texture_program;
    wizard_drawable->pipeline.type = mesh.type;
    wizard_drawable->pipeline.start = mesh.start;
    wizard_drawable->pipeline.count = mesh.count;
    wizard_drawable->specular_info.shininess = 10.0f;
    wizard_drawable->specular_info.specular_brightness = glm::vec3(1.0f, 0.9f, 0.7f);
    
    initialize_scene_metadata();
    initialize_collider("col_", artworld_meshes);
    initialize_wireframe_objects("col_wire");
    initialize_text_collider("text_", artworld_meshes);
}

PlayMode::~PlayMode() = default;

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    if (evt.type == SDL_KEYDOWN) {
        if (text_display.active) {
            text_display.deactivate();
        }
        Command command = terminal.handle_key(evt.key.keysym.sym);
        if (command != Command::False) {
            switch (command) {
                case Command::False:
                    assert(false && "impossible");
                    break;
                case Command::True:
                    break;
                case Command::OpenSesame:
                    unlock("unlock_");
                    std::cout << "command was open sesame!\n";
                    break;
                case Command::Mirage:
                    //activate paintbrush
                {
                    std::string pb_object_name = "col_wire_off_block_Paintbrush";
                    if (!has_paint_ability) {
                        auto pb = scene.collider_name_map[pb_object_name];
                        
                        float distance = pb->min_distance(scene.collider_name_map[player.name]);
                        
                        if (distance < 10) {
                            auto d = scene.drawble_name_map[pb_object_name];
                            assert(d->wireframe_info.draw_frame);
                            d->wireframe_info.draw_frame = false;
                            has_paint_ability = true;
                            scene.colliders.push_back(pb);
                            current_wireframe_objects_map.erase(pb_object_name);
                            if (d->wireframe_info.one_time_change) {
                                wireframe_objects.remove(pb);
                                wf_obj_block_map.erase(pb_object_name);
                                wf_obj_pass_map.erase(pb_object_name);
                                
                            }
                        }
                        
                    }
                }
                    //update_wireframe();
                    std::cout << "command was open mirage!\n";
                    break;
            }
            return true;
        } else if (evt.key.keysym.sym == SDLK_ESCAPE) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
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
        } else if (evt.key.keysym.sym == SDLK_e) {
            terminal.activate();
        } else if (evt.key.keysym.sym == SDLK_r) {
            read.downs += 1;
            read.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_SPACE) {
            std::shared_ptr<Scene::Collider> c = nullptr;
            float distance = 0.0;
            
            std::tie(c, distance) = mouse_collider_check();
            if (c) {
                auto player_collider = scene.collider_name_map[player.name];
                if (distance < 10.0f) {
                    // Do not update if player intersects the object
                    if (!player_collider->intersect(c))
                        update_wireframe(c);
                }
            }
            
            //update_wireframe();
            return true;
        } else if (evt.key.keysym.sym == SDLK_c) {
            std::shared_ptr<Scene::Collider> c = nullptr;
            float distance = 0.0;
            std::tie(c, distance) = mouse_text_check();
            if (c) {
                if (text_storage->object_text_map.count(c->name)) {
                    auto v = text_storage->object_text_map.at(c->name);
                    text_display.add_text(v[0]);
                    text_display.activate();
                }
            }
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
        } else if (evt.key.keysym.sym == SDLK_r) {
            read.pressed = false;
            return true;
        }
    } else if (evt.type == SDL_MOUSEBUTTONDOWN) {
        if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            return true;
        }
    } else if (evt.type == SDL_MOUSEMOTION) {
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
            pitch = std::min(pitch, 0.60f * glm::pi<glm::float32>());
            pitch = std::max(pitch, 0.05f * glm::pi<glm::float32>());
            player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
/*
            const glm::float32 DIST_TO_PLAYER = glm::length(glm::vec3(-0.0f, -5.0f, 2.5f));
            player.camera->transform->position =
                    -player.camera->transform->rotation * glm::vec3(0.0f, 2.0f, DIST_TO_PLAYER);
*/
            
            return true;
        }
    }
    
    return false;
}

void PlayMode::update(float elapsed) {
    if (!animated && read.pressed && animationTime == 0.0) {
      //float distance = std::numeric_limits<float>::max();
      float distance = 5.0; // sight distance
      auto playerToWorld = player.transform->make_local_to_world();
      auto here = player.transform->position;
std::cout << "here " << here.x << " " << here.y << " " << here.z << std::endl;
      std::string selected;
      for (const auto &[name, mesh] : textBearers) {
        if (!endsWith(name, "_m")) {
          continue;
        }
	auto transform = nameToTransform[name];
        auto there = transform->make_local_to_world() * glm::vec4(transform->position, 1.0);
std::cout << "there " << there.x << " " << there.y << " " << there.z << std::endl;
        float newdistance = glm::distance(here, there);
std::cout << "distance: " << newdistance << " name: " << name << std::endl;
        if (newdistance < distance) {
          distance = newdistance;
          selected = name;
        }
      }
      if (selected.size() != 0) {
std::cout << "selected: " << selected << std::endl;
        assert(selected.back() == 'm');
        std::string selectedCamera = textBearerCams[selected];
        auto destCamera = scene.cams[selectedCamera];
        assert(destCamera != nullptr);
        animated = true;
        animationTime = 0.0f;
        auto selectedToWorld = nameToTransform[selected]->make_local_to_world();
        auto endposition = selectedToWorld * glm::vec4(destCamera->transform->position, 1.0);
        //auto playerCameraToWorld = player.camera->transform->make_local_to_world();
        auto startposition = playerToWorld * glm::vec4(player.camera->transform->position, 1.0);
        auto endrotation = glm::quat_cast(glm::mat3(selectedToWorld) * glm::mat3_cast(destCamera->transform->rotation));
        auto startrotation = glm::quat_cast(glm::mat3(playerToWorld) * glm::mat3_cast(player.camera->transform->rotation));
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
    if (animated) {
      animationTime += elapsed;
      animationTime = std::min(1.0f, animationTime);
      player.camera->transform->position = splineposition.at(animationTime);
      player.camera->transform->rotation = splinerotation.at(animationTime);
      if (animationTime == 1.0f) {
        animated = false;
      }
    }

    // reset camera
    if (!animated && !read.pressed && animationTime > 0.0) {
      animationTime = 0.0;
      player.camera->transform->position = glm::vec3(-0.0f, -5.0f, 2.5f);
      player.camera->transform->rotation = glm::vec3(glm::radians(84.0f), glm::radians(0.0f), glm::radians(0.0f));
      // back to player local camera
      player.camera->transform->parent = player.transform;
    }

    //player walking:
    {
        //combine inputs into a move:
        constexpr float PlayerSpeed = 3.0f;
        auto move = glm::vec2(0.0f);
        if (left.pressed && !right.pressed) move.x = -1.0f;
        if (!left.pressed && right.pressed) move.x = 1.0f;
        if (down.pressed && !up.pressed) move.y = -1.0f;
        if (!down.pressed && up.pressed) move.y = 1.0f;
        
        //make it so that moving diagonally doesn't go faster:
        if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
        
        //get move in world coordinate system:
        glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);
        
        //Collision
        {
            auto c = scene.collider_name_map[player.name];
            bool has_collision = false;
            
            
            // If there is collision, reverse the remain vector at the collision direction?
            int idx = -1;
            float overlap = std::numeric_limits<float>::infinity();
            
            
            for (auto collider: scene.colliders) {
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
    }
    
    auto bbox = scene.collider_name_map[player.name];
    bbox->update_BBox(player.transform);
    
    //reset button press counters:
    left.downs = 0;
    right.downs = 0;
    up.downs = 0;
    down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
    //update camera aspect ratio for drawable:
    player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);
    
    //set up light type and position for lit_color_texture_program:
    // TODO: consider using the Light(s) in the scene to do this
//    glUseProgram(lit_color_texture_program->program);
//    glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
//    glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1,
//                 glm::value_ptr(glm::normalize(glm::vec3(0.5f, 1.0f, -1.0f))));
//    glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(0.85f, 0.85f, 0.85f)));
//    glUniform3fv(lit_color_texture_program->AMBIENT_LIGHT_ENERGY_vec3, 1,
//                 glm::value_ptr(glm::vec3(0.25f, 0.25f, 0.25f)));
//    glUseProgram(0);
    
    glUseProgram(rocket_color_texture_program->program);
    glUniform1i(rocket_color_texture_program->LIGHT_TYPE_int, 1);
    glUniform3fv(rocket_color_texture_program->LIGHT_DIRECTION_vec3, 1,
                 glm::value_ptr(glm::normalize(glm::vec3(0.5f, 1.0f, -1.0f))));
    glUniform3fv(rocket_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(0.85f, 0.85f, 0.85f)));
    glUniform3fv(rocket_color_texture_program->AMBIENT_LIGHT_ENERGY_vec3, 1,
                 glm::value_ptr(glm::vec3(0.25f, 0.25f, 0.25f)));
    glUseProgram(0);
    
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
    glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.
    
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    scene.draw(*player.camera, false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    scene.draw(*player.camera, true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    
    // {
    //     DrawLines lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
    //     for(auto r : rays){
    //         lines.draw(r.first,r.second);
    //     }
    // }
    
    
    terminal.draw();
    text_display.draw();
    
    GL_ERRORS();
}


// TODO exclude player collider?
void PlayMode::update_wireframe(std::shared_ptr<Scene::Collider> c) {
    if (!has_paint_ability) {
        return;
    }
    
    
    if (c->name.find("wire") == std::string::npos) {
        return;
    }
    
    {
        bool found = false;
        for (auto it: wireframe_objects) {
            if (it->name == c->name) {
                found = true;
            }
        }
        if (!found)
            return;
    }
    
    
    if (!has_paint_ability) {
        if (c->name.find("Paintbrush") == std::string::npos) {
            return;
        }
    }
    
    bool is_current_wireframe = scene.drawble_name_map[c->name]->wireframe_info.draw_frame;
    auto d = scene.drawble_name_map[c->name];
    
    if (is_current_wireframe) {
        current_wireframe_objects_map.erase(c->name);
        if (wf_obj_block_map.count(c->name)) {
            scene.colliders.push_back(c);
        } else if (wf_obj_pass_map.count(c->name)) {
            scene.colliders.remove(c);
        } else {
            std::runtime_error("Run wireframe state");
        }
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            wireframe_objects.remove(c);
            wf_obj_block_map.erase(c->name);
            wf_obj_pass_map.erase(c->name);
        }
        d->wireframe_info.draw_frame = false;
    } else {
        // remove bounding box
        if (wf_obj_block_map.count(c->name)) {
            scene.colliders.remove(c);
        } else if (wf_obj_pass_map.count(c->name)) {
            scene.colliders.push_back(c);
        }
        current_wireframe_objects_map[c->name] = c;
        
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            wireframe_objects.remove(c);
            wf_obj_block_map.erase(c->name);
            wf_obj_pass_map.erase(c->name);
            current_wireframe_objects_map.erase(c->name);
        }
        d->wireframe_info.draw_frame = true;
    }
    
    if (c->name.find("Paintbrush") != std::string::npos) {
        has_paint_ability = true;
    }
}

void PlayMode::update_wireframe() {
    std::string name_to_real, name_to_wireframe;
    std::shared_ptr<Scene::Collider> collider_to_real = nullptr;  // Add back to fully draw
    std::shared_ptr<Scene::Collider> collider_to_wireframe = nullptr; // draw wireframe
    
    // Test the frame thing?
    
    auto c = scene.collider_name_map[player.name];
    
    if (has_paint_ability) {
        // remove real object, only draw wireframe
        for (auto it = wireframe_objects.begin(); it != wireframe_objects.end(); it++) {
            auto collider = *it;
            if (collider->name == player.name) {
                continue;
            }
            auto dist = c->min_distance(collider);
            if (dist < 0.5) {
                std::string name = collider->name;
                // If this is already a wireframe
                if (!current_wireframe_objects_map.count(name)) {
                    collider_to_wireframe = collider;
                    name_to_wireframe = name;
                    break;
                    
                }
            }
        }
        // turn wireframe object real
        if (collider_to_wireframe == nullptr) {
            // Add it back
            for (auto &it: current_wireframe_objects_map) {
                std::string name = it.first;
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
        for (auto it = wireframe_objects.begin(); it != wireframe_objects.end(); it++) {
            auto collider = *it;
            if (collider->name == player.name || collider->name.find("Paintbrush") == std::string::npos) {
                continue;
            }
            auto dist = c->min_distance(collider);
            if (dist < 0.5) {
                std::string name = collider->name;
                // If this is already a wireframe
                if (!current_wireframe_objects_map.count(name)) {
                    collider_to_wireframe = collider;
                    name_to_wireframe = name;
                    has_paint_ability = true;
                    break;
                    
                }
            }
        }
        if (collider_to_wireframe == nullptr) {
            // Add it back
            for (auto &it: current_wireframe_objects_map) {
                std::string name = it.first;
                if (name.find("Paintbrush") == std::string::npos) {
                    continue;
                }
                
                auto collider = it.second;
                auto dist = c->min_distance(collider);
                if (dist < 0.5 && !c->intersect(collider)) {
                    collider_to_real = collider;
                    name_to_real = name;
                    has_paint_ability = true;
                    break;
                }
            }
            
        }
    }
    
    
    if (collider_to_real) {
        // Add back bounding box
        if (wf_obj_block_map.count(name_to_real)) {
            scene.colliders.push_back(collider_to_real);
        }
            // remove virtual bounding box
        else if (wf_obj_pass_map.count(name_to_real)) {
            scene.colliders.remove(collider_to_real);
        }
        
        current_wireframe_objects_map.erase(name_to_real);
        auto d = scene.drawble_name_map[name_to_real];
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            wireframe_objects.remove(collider_to_real);
            wf_obj_block_map.erase(name_to_real);
            wf_obj_pass_map.erase(name_to_real);
        }
        d->wireframe_info.draw_frame = false;
    }
    
    if (collider_to_wireframe) {
        // remove bounding box
        if (wf_obj_block_map.count(name_to_wireframe)) {
            scene.colliders.remove(collider_to_wireframe);
        } else if (wf_obj_pass_map.count(name_to_wireframe)) {
            scene.colliders.push_back(collider_to_wireframe);
        }
        
        
        current_wireframe_objects_map[name_to_wireframe] = collider_to_wireframe;
        auto d = scene.drawble_name_map[name_to_wireframe];
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change) {
            wireframe_objects.remove(collider_to_wireframe);
            wf_obj_block_map.erase(name_to_wireframe);
            wf_obj_pass_map.erase(name_to_wireframe);
            current_wireframe_objects_map.erase(name_to_wireframe);
        }
        d->wireframe_info.draw_frame = true;
    }
}


// prefix_on(off)_(onetime)_xxxxx
// on means draw full color at first
// check if there is a prefix_on(off)_(onetime)_xxxxx_invisible
void PlayMode::initialize_wireframe_objects(std::string prefix) {
    for (auto &c: scene.colliders) {
        if (c->name.find(prefix) != std::string::npos) {
            wireframe_objects.push_back(c);
            // Only one time?
            auto d = scene.drawble_name_map[c->name];
            
            if (c->name.find("pass") != std::string::npos) {
                //wf_obj_pass.push_back(c);
                wf_obj_pass_map[c->name] = c;
            } else if (c->name.find("block") != std::string::npos) {
                //wf_obj_block.push_back(c);
                wf_obj_block_map[c->name] = c;
            } else {
                std::runtime_error("Unknown type of wireframe object");
            }
            
            if (c->name.find("onetime") != std::string::npos) {
                d->wireframe_info.one_time_change = true;
            } else {
                d->wireframe_info.one_time_change = false;
            }
            if (c->name.find("on") != std::string::npos) {
                d->wireframe_info.draw_frame = false;
            } else {
                d->wireframe_info.draw_frame = true;
                current_wireframe_objects_map[c->name] = c;
            }
        }
    }
    
    // remove colliders in wf_obj_block_map && colliders is currently wireframe
    // remove colliders in wf_obj_pass_map && colliders is currently real
    for (auto it: wf_obj_block_map) {
        auto d = scene.drawble_name_map[it.second->name];
        if (d->wireframe_info.draw_frame == true) {
            scene.colliders.remove(it.second);
        }
    }
    
    for (auto it: wf_obj_pass_map) {
        auto d = scene.drawble_name_map[it.second->name];
        if (d->wireframe_info.draw_frame == false) {
            scene.colliders.remove(it.second);
        }
    }
}

// Should be called after all drawables are loaded into the list
void PlayMode::initialize_scene_metadata() {
    std::shared_ptr<Scene::Drawable> walkmesh_to_remove = nullptr;
    for (auto d: scene.drawables) {
        std::string name = d->transform->name;
        if (name == "WalkMesh") {
            walkmesh_to_remove = d;
        } else {
            scene.drawble_name_map[name] = d;
        }
    }
    
    if (walkmesh_to_remove) {
        scene.drawables.remove(walkmesh_to_remove);
    }
}


// Which mesh to lookup?
// prefix_xxxxx
void PlayMode::initialize_collider(std::string prefix, Load<MeshBuffer> meshes) {
    for (auto &it: meshes->meshes) {
        std::string name = it.first;
        auto mesh = it.second;
        if (name.find(prefix) != std::string::npos || name == "Player") {
            glm::vec3 min = mesh.min;
            glm::vec3 max = mesh.max;
            auto collider = std::make_shared<Scene::Collider>(name, min, max, min, max);
            auto d = scene.drawble_name_map[name];
            collider->update_BBox(d->transform);
            scene.colliders.push_back(collider);
            scene.collider_name_map[name] = collider;
        }
    }
}


void PlayMode::initialize_text_collider(std::string prefix, Load<MeshBuffer> meshes) {
    for (auto &it: meshes->meshes) {
        std::string name = it.first;
        auto mesh = it.second;
        if (name.find(prefix) != std::string::npos) {
            glm::vec3 min = mesh.min;
            glm::vec3 max = mesh.max;
            auto collider = std::make_shared<Scene::Collider>(name, min, max, min, max);
            auto d = scene.drawble_name_map[name];
            if (d == nullptr) {
              continue;
            }
            collider->update_BBox(d->transform);
            scene.text_colliders.push_back(collider);
            scene.textcollider_name_map[name] = collider;
        }
    }
}


// Item to unlock must be a collider
void PlayMode::unlock(std::string prefix) {
    
    auto c = scene.collider_name_map[player.name];
    
    std::shared_ptr<Scene::Collider> collider_to_remove = nullptr;
    std::string name_to_remove;
    
    for (auto collider: scene.colliders) {
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
    auto d = scene.drawble_name_map[name_to_remove];
    scene.drawables.remove(d);
    scene.drawble_name_map.erase(name_to_remove);
    scene.colliders.remove(collider_to_remove);
    scene.collider_name_map.erase(name_to_remove);
}


std::pair<std::shared_ptr<Scene::Collider>, float> PlayMode::mouse_text_check(std::string prefix) {
    if (SDL_GetRelativeMouseMode() != SDL_FALSE)
        return std::make_pair(nullptr, 0);
    
    int x, y;
    SDL_GetMouseState(&x, &y);
    
    y = 720 - y;
    
    float ux = (x - 640.0) / 640.0;
    float uy = (y - 360.0) / 360.0;
    
    
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
    
    for (auto it: scene.textcollider_name_map) {
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

std::pair<std::shared_ptr<Scene::Collider>, float> PlayMode::mouse_collider_check(std::string prefix) {
    if (SDL_GetRelativeMouseMode() != SDL_FALSE)
        return std::make_pair(nullptr, 0);
    
    int x, y;
    SDL_GetMouseState(&x, &y);
    
    y = 720 - y;
    
    float ux = (x - 640.0) / 640.0;
    float uy = (y - 360.0) / 360.0;
    
    
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
    
    for (auto it: scene.collider_name_map) {
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
        
        float distance = glm::length(dir.d * dir.t);
        return std::make_pair(intersected_collider, distance);
    }
    
    return std::make_pair(nullptr, 0.0f);
}

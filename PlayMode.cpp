#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Components/EventHandler.hpp"
#include "spline.h"


#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>

GLuint phonebank_meshes_for_lit_color_texture_program = 0;
GLuint wizard_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> phonebank_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("phone-bank.pnct"));
    phonebank_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    return ret;
});

Load<MeshBuffer> wizard_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("wizard.pnct"));
    wizard_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    return ret;
});

Load<Scene> phonebank_scene(LoadTagDefault, []() -> Scene const * {
    return new Scene(
            data_path("phone-bank.scene"),
            [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
                Mesh const &mesh = phonebank_meshes->lookup(mesh_name);
                
                scene.drawables.emplace_back( std::make_shared<Scene::Drawable>(transform));
                std::shared_ptr<Scene::Drawable> &drawable = scene.drawables.back();
                
                drawable->pipeline = lit_color_texture_program_pipeline;
                
                drawable->pipeline.vao = phonebank_meshes_for_lit_color_texture_program;
                drawable->pipeline.type = mesh.type;
                drawable->pipeline.start = mesh.start;
                drawable->pipeline.count = mesh.count;
                
            });
});

WalkMesh const *walkmesh = nullptr;
Load<WalkMeshes> phonebank_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
    auto *ret = new WalkMeshes(data_path("phone-bank.w"));
    walkmesh = &ret->lookup("WalkMesh");
    return ret;
});

PlayMode::PlayMode() : scene(*phonebank_scene) {
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
    player.transform = &scene.transforms.back();
    
    //create a player camera attached to a child of the player transform:
    scene.transforms.emplace_back();
    scene.cameras.emplace_back(&scene.transforms.back());
    player.camera = &scene.cameras.back();
    player.camera->fovy = glm::radians(60.0f);
    player.camera->near = 0.01f;
    player.camera->transform->parent = player.transform;
    
    //default view point behind player
    player.camera->transform->position = glm::vec3(-2.5f, -5.0f, 2.5f);
    
    //rotate camera to something pointing in way of player
    // arcsin 0.1 ~ 6 degrees
    player.camera->transform->rotation = glm::vec3(glm::radians(84.0f), glm::radians(0.0f), glm::radians(0.0f));
    //glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    //start player walking at nearest walk point:
    player.at = walkmesh->nearest_walk_point(player.transform->position);

    //scene.transforms.emplace_back();
    //auto transform = &scene.transforms.back();
    Scene::Transform *transform = player.transform;
    //transform->scale *= 2.0f;
    Mesh const &mesh = wizard_meshes->lookup("wizard");
    scene.drawables.emplace_back( std::make_shared<Scene::Drawable>(transform));
    std::shared_ptr<Scene::Drawable> &drawable = scene.drawables.back();

    drawable->pipeline = lit_color_texture_program_pipeline;

    drawable->pipeline.vao = wizard_meshes_for_lit_color_texture_program;
    drawable->pipeline.type = mesh.type;
    drawable->pipeline.start = mesh.start;
    drawable->pipeline.count = mesh.count;
}

PlayMode::~PlayMode() = default;

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    
    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.sym == SDLK_ESCAPE) {
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
            pitch = std::min(pitch, 0.95f * 3.1415926f);
            pitch = std::max(pitch, 0.05f * 3.1415926f);
            player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            
            return true;
        }
    }
    
    return false;
}

void PlayMode::update(float elapsed) {
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
                    walkmesh->to_world_smooth_normal(player.at) //smoothed up vector at walk location
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
    glUseProgram(lit_color_texture_program->program);
    glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
    glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
    glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
    glUseProgram(0);
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.


	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	scene.draw(*player.camera, false);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	scene.draw(*player.camera, true);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    
    /* In case you are wondering if your walkmesh is lining up with your scene, try:
    {
        glDisable(GL_DEPTH_TEST);
        DrawLines lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
        for (auto const &tri : walkmesh->triangles) {
            lines.draw(walkmesh->vertices[tri.x], walkmesh->vertices[tri.y], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
            lines.draw(walkmesh->vertices[tri.y], walkmesh->vertices[tri.z], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
            lines.draw(walkmesh->vertices[tri.z], walkmesh->vertices[tri.x], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
        }
    }
    */
    
    { //use DrawLines to overlay some text:
        glDisable(GL_DEPTH_TEST);
        float aspect = float(drawable_size.x) / float(drawable_size.y);
        DrawLines lines(glm::mat4(
                1.0f / aspect, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        ));
        
        constexpr float H = 0.09f;
        lines.draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
                        glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0x00, 0x00, 0x00, 0x00));
        float ofs = 2.0f / drawable_size.y;
        lines.draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
                        glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
    }
    GL_ERRORS();
}



void PlayMode::update_wireframe(){
    std::string name_to_add, name_to_remove;
    std::shared_ptr<Scene::Collider> collider_to_add = nullptr;  // Add back to fully draw
    std::shared_ptr<Scene::Collider> collider_to_remove = nullptr; // draw wireframe

    // Test the frame thing?
    if (use.downs > 0 && !use.pressed){
        use.downs = 0;
        auto c = scene.collider_name_map[player.name];
        // TODO, use a interactable object list, because there is no such list, need to use two seprate loops for now

        // remove object, only draw wireframe
        for (auto it = wireframe_objects.begin(); it != wireframe_objects.end(); it ++){
            auto collider = *it;
            if(collider->name == player.name){
                continue;
            }
            auto dist = c->min_distance(collider);
            if (dist < 0.5){
                std::string name = collider->name;
                // If this is already a wireframe
                if (!current_wireframe_objects_map.count(name)){
                    collider_to_remove = collider;
                    name_to_remove = name;
                    use.downs = 0;
                    break;
                
                }   
            }
        }
        if (collider_to_remove == nullptr){
            // Add it back
            for(auto &it : current_wireframe_objects_map){
                std::string name = it.first;
                auto collider = it.second;
                auto dist = c->min_distance(collider);
                if (dist < 0.5 && !c->intersect(collider)){
                    collider_to_add = collider;
                    name_to_add = name;
                    use.downs = 0;
                    break;
                }
            }

        }

    }

    if(collider_to_add){
        scene.colliders.push_back(collider_to_add);
        current_wireframe_objects_map.erase(name_to_add);
        auto d = scene.drawble_name_map[name_to_add];

        // If first_time_add/remove
        if (d->wireframe_info.one_time_change){
            wireframe_objects.remove(collider_to_add);
        }
        d->wireframe_info.draw_frame = false;
    }

    if(collider_to_remove){
        scene.colliders.remove(collider_to_remove);
        current_wireframe_objects_map[name_to_remove] = collider_to_remove;
        auto d = scene.drawble_name_map[name_to_remove];
        // If first_time_add/remove
        if (d->wireframe_info.one_time_change){
            wireframe_objects.remove(collider_to_add);
        }
        d->wireframe_info.draw_frame = true;
    }
}
#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Components/EventHandler.hpp"

#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "WalkMesh.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>
#include <array>

GLuint meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("mesh.pnct"));
	meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > mesh_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("mesh.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = meshes->lookup(mesh_name);

		scene.drawables.emplace_back(new Scene::Drawable(transform));
		std::shared_ptr<Scene::Drawable> drawable = scene.drawables.back();

		drawable->pipeline = lit_color_texture_program_pipeline;

		drawable->pipeline.vao = meshes_for_lit_color_texture_program;
		drawable->pipeline.type = mesh.type;
		drawable->pipeline.start = mesh.start;
		drawable->pipeline.count = mesh.count;

	});
});

WalkMesh const *walkmesh = nullptr;
Load< WalkMeshes > walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
	WalkMeshes *ret = new WalkMeshes(data_path("mesh.w"));
	walkmesh = &ret->lookup("walkmesh");
	return ret;
});


PlayMode::PlayMode() : scene(*mesh_scene) {

    // Build AABB bounding boxes
    for(auto &t : scene.transforms){
        auto name = t.name;
        //We don't need these bounding boxes
        if(name == "Point" || name == "Camera" || name == "walkmesh")
            continue;
        auto mesh = meshes->lookup(name);
        // Make it a little bigger?
        std::shared_ptr<Scene::Collider> collider = std::make_shared<Scene::Collider>(name,mesh.min,mesh.max,mesh.min,mesh.max);
        collider->update_BBox(&t);
        scene.colliders.push_back(collider);
        scene.collider_name_map[name] = collider;
    }



    for(auto &d : scene.drawables){
        std::string name = d->transform->name;
        scene.drawble_name_map[name] = d;
    }


        // Build wireframe obj list
    for(auto & c : scene.colliders){
        if(c->name == "obstacle"){
            wireframe_objects.push_back(c);
        }

        if(c->name == "door"){
            wireframe_objects.push_back(c);
            auto d = scene.drawble_name_map[c->name];
            current_wireframe_objects_map[c->name] = c;
            d->wireframe_info.draw_frame = true;
            d->wireframe_info.one_time_change = true;
        }
    }
    
    auto c = scene.collider_name_map["door"];
    scene.colliders.remove(c);



	//create a player transform:
	//scene.transforms.emplace_back();
	for(auto &t : scene.transforms){
		if (t.name == "player"){
			player.transform = &t;
			player.original_transform = new Scene::Transform();
			*player.original_transform = t;
		}
	}


	//create a player camera attached to a child of the player transform:
	scene.transforms.emplace_back();
	scene.cameras.emplace_back(&scene.transforms.back());
	player.camera = &scene.cameras.back();

	player.camera->fovy = glm::radians(60.0f);
	player.camera->near = 0.01f;
	player.camera->transform->parent = player.transform;

	//debug
	//player.transform->position = glm::vec3(0.6,8.7,1.9);

	player.camera->transform->position = glm::vec3(-2.0f, 0.0f, 2.0f);

	//rotate camera facing direction (-z) to player facing direction (-x):
	player.camera->transform->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	player.camera->transform->rotation *= glm::angleAxis(glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));


	//start player walking at nearest walk point:
	player.at = walkmesh->nearest_walk_point(player.transform->position);

}




PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    
    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.repeat) {
            //ignore repeats
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
        } else if (evt.key.keysym.sym == SDLK_SPACE) {
            jump.downs += 1;
            jump.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_e){
            use.downs += 1;
            use.pressed = true;
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
        } else if (evt.key.keysym.sym == SDLK_SPACE) {
            jump.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_e){
            use.pressed = false;
            return true;
        }
    }else if (evt.type == SDL_MOUSEBUTTONDOWN) {
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

            auto rotation = glm::angleAxis(-motion.x * player.camera->fovy, upDir);

			player.transform->rotation = rotation  * player.transform->rotation;
            player.direction = rotation * glm::vec4(player.direction,0.0);
			return true;
		}
	}
    
    return false;
}

void PlayMode::update(float elapsed) {    
    constexpr float PlayerSpeed = 3.0f;
    glm::vec2 move = glm::vec2(0.0f);
    if (left.pressed && !right.pressed) move.x = -1.0f;
    if (!left.pressed && right.pressed) move.x = 1.0f;
    if (down.pressed && !up.pressed) move.y = -1.0f;
    if (!down.pressed && up.pressed) move.y = 1.0f;

    //make it so that moving diagonally doesn't go faster:
    if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;


    glm::vec3 dir = player.direction;
    glm::vec3 normal = walkmesh->to_world_triangle_normal(player.at);
    glm::vec3 right_vec = glm::cross(dir,normal);

    glm::vec3 remain = move.x * right_vec + move.y * dir; 


    update_wireframe();



    // Collision
    {

        auto c = scene.collider_name_map[player.name];
        bool has_collision = false;


        // If there is collision, reverse the remain vector at the collision direction?
        int idx = -1;
        float overlap = std::numeric_limits<float>::infinity();



        for( auto collider : scene.colliders){
            if (collider->name == player.name){
                continue;
            }else{
                if (c->intersect(collider)){
                    has_collision = true;
                    // Only one collision at a time?
                    std::tie(idx,overlap) = c->least_collison_axis(collider);
                    break;
                }
            }
        }

        if (has_collision){
            remain[idx] += overlap;
        }


    }


    // update player direction?
    {
        //https://iquilezles.org/articles/noacos/
        [[maybe_unused]] auto rotation_align = [](glm::vec3 &d, glm::vec3 &z)->glm::mat3x3{
            const glm::vec3  v = cross( z, d );
            const float c = dot( z, d );
            const float k = 1.0f/(1.0f+c);

            return glm::mat3x3( v.x*v.x*k + c,     v.y*v.x*k - v.z,    v.z*v.x*k + v.y,
                        v.x*v.y*k + v.z,   v.y*v.y*k + c,      v.z*v.y*k - v.x,
                        v.x*v.z*k - v.y,   v.y*v.z*k + v.x,    v.z*v.z*k + c    );
        };
       
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
            glm::vec3 along = glm::normalize(b-a);
            glm::vec3 normal = glm::normalize(glm::cross(b-a, c-a));
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

    auto bbox = scene.collider_name_map[player.name];
    bbox->update_BBox(player.transform);

    //reset button press counters:
    left.downs = 0;
    right.downs = 0;
    up.downs = 0;
    down.downs = 0;
    jump.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
    
   //update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
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


    // Debug bounding box
    {
        DrawLines draw_lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
        //draw bounding box:
        for(auto collider : scene.colliders){
            auto current_mesh_max = collider->max;
            auto current_mesh_min = collider->min;

            glm::vec3 r = 0.5f * (current_mesh_max - current_mesh_min);
            glm::vec3 c = 0.5f * (current_mesh_max + current_mesh_min);
            glm::mat4x3 mat(
                glm::vec3(r.x,  0.0f, 0.0f),
                glm::vec3(0.0f,  r.y, 0.0f),
                glm::vec3(0.0f, 0.0f,  r.z),
                c
            );
            draw_lines.draw_box(mat, glm::u8vec4(0xdd, 0xdd, 0xdd, 0xff));
        }
		
    }

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		std::string text;

		// if(!success && !failed){
		// 	text = "WS moves; AD turns";
		// }else if (success){
		// 	char out[100];
		// 	auto length = sprintf(out,"Congrats! You Win! Game will end in %.1f seconds",exit_timer - elapsed_time.count());
		// 	text = std::string(out,length);
		// }else if (failed){
		// 	char out[100];
		// 	auto length = sprintf(out,"You failed! Game will end in %.1f seconds",exit_timer - elapsed_time.count());
		// 	text = std::string(out,length);
		// }
        text = "WASD moves;";

		constexpr float H = 0.09f;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
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
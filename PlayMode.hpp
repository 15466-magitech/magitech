#include "Mode.hpp"

#include "Scene.hpp"

#include "Connection.hpp"

#include "WalkMesh.hpp"

#include "Collision.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	Scene scene;

	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, jump, use;


	struct Player {
		WalkPoint at;
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
		//camera is at player's head and will be pitched by mouse up/down motion:
		Scene::Camera *camera = nullptr;
		glm::vec3 speed{-0.01f,0.0f,0.0f};
		bool on_walkmesh = true;
		float max_speed = 2.0f;
		glm::vec3 direction = {1.0f,0.0f,0.0f};
		Scene::Transform *original_transform = nullptr;
		uint8_t reset_time = 3;
		std::string name = "player";
	} player;


	//----- game state -----
	std::list<std::shared_ptr<Scene::Collider>> wireframe_objects;
	std::unordered_map<std::string, std::shared_ptr<Scene::Collider>> current_wireframe_objects_map;
	void update_wireframe();


	//last message from server:
	std::string server_message;


};

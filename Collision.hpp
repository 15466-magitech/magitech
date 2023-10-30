#include "Mesh.hpp"
#include "Scene.hpp"
#include <vector>

struct Collider{

		std::string name;

		Collider(std::string name, glm::vec3 min, glm::vec3 max){
			this->min = min;
			this->max = max;
			this->name = name;
		}


		glm::vec3 min = glm::vec3( std::numeric_limits< float >::infinity());
		glm::vec3 max = glm::vec3(-std::numeric_limits< float >::infinity());
		
		
		bool intersect(Collider c);

		bool point_intersect(glm::vec3 point);

		std::vector<glm::vec3> get_vertices();

		void update_BBox(Scene::Transform t);

	};
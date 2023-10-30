#include "Collision.hpp"


bool Collider::intersect(Collider c){
    if (
        min.x <= c.max.x &&
        max.x >= c.min.x &&
        min.y <= c.max.y &&
        max.y >= c.min.y &&
        min.z <= c.max.z &&
        max.z >= c.min.z
     )
        return true;
     else
        return false;

}


bool Collider::point_intersect(glm::vec3 p){
    if (
        p.x > min.x &&
        p.x < max.x &&
        p.y > min.y &&
        p.y < max.y &&
        p.z > min.z &&
        p.z < max.z
    )
        return true;
    else
        return false;
}


std::vector<glm::vec3> Collider::get_vertices(){
    std::vector<float> xs {min.x, max.x};
    std::vector<float> ys {min.y, max.y};
    std::vector<float> zs {min.z, max.z};

    std::vector<glm::vec3> ret;

    for (uint8_t i = 0; i < xs.size(); i++){
        for (uint8_t j = 0; j < ys.size(); j++){
            for (uint8_t k = 0; k < zs.size(); k++){
                glm::vec3 tmp = glm::vec3(xs[i],ys[j],zs[k]);
            }
        }
    }

    return ret;
}


void Collider::update_BBox(Scene::Transform t){

    std::vector<glm::vec3> current_vertices = get_vertices();
    //std::vector<glm::vec3> new_vertices;

    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> zs;

    auto trans = t.make_local_to_world();

    for (auto v : current_vertices){
        auto newVertex = trans * glm::vec4{v,1};
        xs.push_back(newVertex.x);
        ys.push_back(newVertex.y);
        zs.push_back(newVertex.z);
    }

    // Make it axis-aligned again
    float minX = std::numeric_limits<float>::max();
    float minY = minX;
    float minZ = minX;

    float maxX = std::numeric_limits<float>::min();
    float maxY = maxX;
    float maxZ = maxX;

    assert(xs.size() == ys.size() && ys.size() == zs.size());
    for (uint8_t i = 0; i < xs.size(); i++){
        if (xs[i] < minX){
            minX = xs[i];
        }
        if (xs[i] > maxX){
            maxX = xs[i];
        }
        if (ys[i] < minY){
            minY = ys[i];
        }
        if (ys[i] > maxY){
            maxY = ys[i];
        }
        if (zs[i] < minZ){
            minZ = zs[i];
        }
        if (zs[i] > maxZ){
            maxZ = zs[i];
        }
    }

    min.x = minX; min.y = minY; min.z = minZ;
    max.x = maxX; max.y = maxY; max.z = maxZ;


}
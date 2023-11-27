#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

struct ShadowMapProgram {
    ShadowMapProgram();
    ~ShadowMapProgram();
    
    GLuint program = 0;
    
    //Attribute (per-vertex variable) locations:
    GLuint Position_vec4 = -1U;
    GLuint Normal_vec3 = -1U;
    GLuint Color_vec4 = -1U;
    GLuint TexCoord_vec2 = -1U;

    //Uniform (per-invocation variable) locations:
    GLuint OBJECT_TO_LIGHT_mat4x3 = -1U;
    GLuint OBJECT_TO_CLIP_mat4 = -1U;
};

extern Load< ShadowMapProgram > shadow_map_program;

extern Scene::Drawable::Pipeline shadow_map_program_pipeline;

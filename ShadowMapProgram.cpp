//
// Created by Russell Emerine on 11/9/23.
//

#include "ShadowMapProgram.hpp"

#include "gl_compile_program.hpp"
#include "glm/ext.hpp"

Scene::Drawable::Pipeline shadow_map_program_pipeline;

Load<ShadowMapProgram> shadow_map_program(LoadTagEarly, []() -> ShadowMapProgram const * {
    ShadowMapProgram *ret = new ShadowMapProgram();
    
    //----- build the pipeline template -----
    shadow_map_program_pipeline.program = ret->program;

    shadow_map_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
    shadow_map_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;

    return ret;
});

ShadowMapProgram::ShadowMapProgram() {
    //Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
    program = gl_compile_program(
            //vertex shader:
            "#version 330\n"
            "uniform mat4 OBJECT_TO_CLIP;\n"
            "uniform mat4x3 OBJECT_TO_LIGHT;\n"
            "uniform mat3 NORMAL_TO_LIGHT;\n"
            "in vec4 Position;\n"
            "in vec3 Normal;\n"
            "in vec4 Color;\n"
            "in vec2 TexCoord;\n"
            "out vec3 position;\n"
            "out vec3 normal;\n"
            "out vec4 color;\n"
            "out vec2 texCoord;\n"
            "void main() {\n"
            "	//position = OBJECT_TO_CLIP * Position;\n"
            "	gl_Position = vec4(OBJECT_TO_LIGHT * Position, 1.0f);\n"
q            "	normal = NORMAL_TO_LIGHT * Normal;\n"
            "	color = Color;\n"
            "	texCoord = TexCoord;\n"
            "}\n",
            //fragment shader:
            "#version 330\n"
            "uniform mat4 OBJECT_TO_CLIP;\n"
            "uniform mat4x3 OBJECT_TO_LIGHT;\n"
            "in vec4 color;\n"
            "out vec4 fragColor;"
            "void main() {\n"
            "   fragColor = color;\n"
            "}\n"
    );

    Position_vec4 = glGetAttribLocation(program, "Position");
    Normal_vec3 = glGetAttribLocation(program, "Normal");
    Color_vec4 = glGetAttribLocation(program, "Color");
    TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

    //look up the locations of uniforms:
    OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
    OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
}

ShadowMapProgram::~ShadowMapProgram() {
    glDeleteProgram(program);
    program = 0;
}


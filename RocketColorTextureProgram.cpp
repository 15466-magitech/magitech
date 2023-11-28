//
// Created by Russell Emerine on 11/9/23.
//

#include "RocketColorTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include "glm/ext.hpp"

Scene::Drawable::Pipeline rocket_color_texture_program_pipeline;

Load<RocketColorTextureProgram> rocket_color_texture_program(LoadTagEarly, []() -> RocketColorTextureProgram const * {
    RocketColorTextureProgram *ret = new RocketColorTextureProgram();
    
    //----- build the pipeline template -----
    rocket_color_texture_program_pipeline.program = ret->program;
    
    rocket_color_texture_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
    rocket_color_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
    rocket_color_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;
    rocket_color_texture_program_pipeline.SPECULAR_BRIGHTNESS_vec3 = ret->SPECULAR_BRIGHTNESS_vec3;
    rocket_color_texture_program_pipeline.SPECULAR_SHININESS_float = ret->SPECULAR_SHININESS_float;
    rocket_color_texture_program_pipeline.draw_frame = ret->draw_frame;
    rocket_color_texture_program_pipeline.set_uniforms = [ret]() {
        glUniform1i(ret->LIGHT_TYPE_int, 1);
        glUniform3fv(ret->LIGHT_DIRECTION_vec3, 1,
                     glm::value_ptr(glm::normalize(glm::vec3(0.5f, 1.0f, -1.0f))));
        glUniform3fv(ret->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(0.85f, 0.85f, 0.85f)));
        glUniform3fv(ret->AMBIENT_LIGHT_ENERGY_vec3, 1,
                     glm::value_ptr(glm::vec3(0.25f, 0.25f, 0.25f)));
    };
    
    /* This will be used later if/when we build a light loop into the Scene:
    rocket_color_texture_program_pipeline.LIGHT_TYPE_int = ret->LIGHT_TYPE_int;
    rocket_color_texture_program_pipeline.LIGHT_LOCATION_vec3 = ret->LIGHT_LOCATION_vec3;
    rocket_color_texture_program_pipeline.LIGHT_DIRECTION_vec3 = ret->LIGHT_DIRECTION_vec3;
    rocket_color_texture_program_pipeline.LIGHT_ENERGY_vec3 = ret->LIGHT_ENERGY_vec3;
    rocket_color_texture_program_pipeline.LIGHT_CUTOFF_float = ret->LIGHT_CUTOFF_float;
    */
    
    //make a 1-pixel white texture to bind by default:
    GLuint tex;
    glGenTextures(1, &tex);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    std::vector<glm::u8vec4> tex_data(1, glm::u8vec4(0xff));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    rocket_color_texture_program_pipeline.textures[0].texture = tex;
    rocket_color_texture_program_pipeline.textures[0].target = GL_TEXTURE_2D;
    
    return ret;
});

RocketColorTextureProgram::RocketColorTextureProgram() {
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
            "	gl_Position = OBJECT_TO_CLIP * Position;\n"
            "	position = OBJECT_TO_LIGHT * Position;\n"
            "	normal = NORMAL_TO_LIGHT * Normal;\n"
            "	color = Color;\n"
            "	texCoord = TexCoord;\n"
            "}\n",
            //fragment shader:
            "#version 330\n"
            "uniform mat4 OBJECT_TO_CLIP;\n"
            "uniform sampler2D TEX;\n"
            "uniform int LIGHT_TYPE;\n"
            "uniform vec3 LIGHT_LOCATION;\n"
            "uniform vec3 LIGHT_DIRECTION;\n"
            "uniform vec3 LIGHT_ENERGY;\n"
            "uniform vec3 AMBIENT_LIGHT_ENERGY;\n"
            "uniform float SPECULAR_SHININESS;\n"
            "uniform vec3 SPECULAR_BRIGHTNESS;\n"
            "uniform float LIGHT_CUTOFF;\n"
            "in vec3 position;\n"
            "in vec3 normal;\n"
            "in vec4 color;\n"
            "in vec2 texCoord;\n"
            "out vec4 fragColor;\n"
            "uniform bool wireframe;\n"
            "// Code adapted from https://github.com/aehmttw/Machimania/blob/master/resources/shaders/main.frag\n"
            "mat3 toMat3(mat4 matrix) {\n"
            "    return mat3(matrix[0].xyz, matrix[1].xyz, matrix[2].xyz);\n"
            "}\n"
            "float det(mat2 matrix) {\n"
            "    return matrix[0].x * matrix[1].y - matrix[0].y * matrix[1].x;\n"
            "}"
            "void main() {\n"
            "	vec3 n = normalize(normal);\n"
            "	vec3 e;\n"
            "	if (LIGHT_TYPE == 0) { //point light \n"
            "		vec3 l = (LIGHT_LOCATION - position);\n"
            "		float dis2 = dot(l,l);\n"
            "		l = normalize(l);\n"
            "		float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
            "		e = nl * LIGHT_ENERGY;\n"
            "	} else if (LIGHT_TYPE == 1) { //hemi light \n"
            "		e = (dot(n,-LIGHT_DIRECTION) * 0.5 + 0.5) * LIGHT_ENERGY + AMBIENT_LIGHT_ENERGY;\n"
            "	} else if (LIGHT_TYPE == 2) { //spot light \n"
            "		vec3 l = (LIGHT_LOCATION - position);\n"
            "		float dis2 = dot(l,l);\n"
            "		l = normalize(l);\n"
            "		float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
            "		float c = dot(l,-LIGHT_DIRECTION);\n"
            "		nl *= smoothstep(LIGHT_CUTOFF,mix(LIGHT_CUTOFF,1.0,0.1), c);\n"
            "		e = nl * LIGHT_ENERGY;\n"
            "	} else { //(LIGHT_TYPE == 3) //directional light \n"
            "		e = max(0.0, dot(n,-LIGHT_DIRECTION)) * LIGHT_ENERGY;\n"
            "	}\n"
            "   vec3 cam = normalize((inverse(toMat3(OBJECT_TO_CLIP)) * vec3(0, 0, 1)).xyz);\n"
            "   vec3 h = normalize(cam + normalize(-LIGHT_DIRECTION));\n"
            "   float specular = pow(max(dot(n, h), 0), SPECULAR_SHININESS);\n" // changes made here
            "   e += specular * SPECULAR_BRIGHTNESS;\n"
            "	vec4 albedo = texture(TEX, texCoord) * color;\n"
            "   if(wireframe){\n"
            "   	fragColor = vec4(0.0,0.0,0.0,1.0);\n"
            "	}\n"
            "	else{\n"
            "		fragColor = vec4(e*albedo.rgb, 1.0);\n"
            "	}\n"
            "}\n"
    );
    //As you can see above, adjacent strings in C/C++ are concatenated.
    // this is very useful for writing long shader programs inline.
    
    //look up the locations of vertex attributes:
    Position_vec4 = glGetAttribLocation(program, "Position");
    Normal_vec3 = glGetAttribLocation(program, "Normal");
    Color_vec4 = glGetAttribLocation(program, "Color");
    TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");
    
    //look up the locations of uniforms:
    OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
    OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
    NORMAL_TO_LIGHT_mat3 = glGetUniformLocation(program, "NORMAL_TO_LIGHT");

    LIGHT_TYPE_int = glGetUniformLocation(program, "LIGHT_TYPE");
    LIGHT_LOCATION_vec3 = glGetUniformLocation(program, "LIGHT_LOCATION");
    LIGHT_DIRECTION_vec3 = glGetUniformLocation(program, "LIGHT_DIRECTION");
    LIGHT_ENERGY_vec3 = glGetUniformLocation(program, "LIGHT_ENERGY");
    AMBIENT_LIGHT_ENERGY_vec3 = glGetUniformLocation(program, "AMBIENT_LIGHT_ENERGY");
    LIGHT_CUTOFF_float = glGetUniformLocation(program, "LIGHT_CUTOFF");
    SPECULAR_BRIGHTNESS_vec3 = glGetUniformLocation(program, "SPECULAR_BRIGHTNESS");
    SPECULAR_SHININESS_float = glGetUniformLocation(program, "SPECULAR_SHININESS");
    
    draw_frame = glGetUniformLocation(program, "wireframe");
    
    
    GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");
    
    //set TEX to always refer to texture binding zero:
    glUseProgram(program); //bind program -- glUniform* calls refer to this program now
    
    glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0
    
    glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

RocketColorTextureProgram::~RocketColorTextureProgram() {
    glDeleteProgram(program);
    program = 0;
}


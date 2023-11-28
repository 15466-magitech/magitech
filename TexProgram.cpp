// From Matei Budiu (mateib)'s Auriga (game4)
#include "TexProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include "glm/ext.hpp"

Scene::Drawable::Pipeline tex_program_pipeline;

Load<TexProgram> tex_program(LoadTagEarly, []() -> TexProgram const * {
    TexProgram *ret = new TexProgram();
    
    //----- build the pipeline template -----
    tex_program_pipeline.program = ret->program;
    
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
    
    tex_program_pipeline.textures[0].texture = tex;
    tex_program_pipeline.textures[0].target = GL_TEXTURE_2D;
    
    return ret;
});

// Inspired by Jim McCann's message in the course Discord on how to create a textured quad:
// https://discord.com/channels/1144815260629479486/1154543452520984686/1156347836888256582
TexProgram::TexProgram() {
    //Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
    program = gl_compile_program(
            //vertex shader:
            "#version 330\n"
            "uniform mat4 OBJECT_TO_CLIP;\n"
            "uniform vec4 COLOR;\n"
            "in vec4 Position;\n"
            "in vec2 TexCoord;\n"
            "out vec2 texCoord;\n"
            "void main() {\n"
            "	gl_Position = OBJECT_TO_CLIP * Position;\n"
            "	texCoord = TexCoord;\n"
            "}\n",
            //fragment shader:
            "#version 330\n"
            "uniform sampler2D TEX;\n"
            "uniform vec4 COLOR;\n"
            "in vec2 texCoord;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "   fragColor = texture(TEX, texCoord) * COLOR;\n"
            "}\n"
    );
    //As you can see above, adjacent strings in C/C++ are concatenated.
    //this is very useful for writing long shader programs inline.
    
    //look up the locations of vertex attributes:
    Position_vec4 = glGetAttribLocation(program, "Position");
    TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");
    
    //look up the locations of uniforms:
    COLOR_vec4 = glGetUniformLocation(program, "COLOR");
    OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
    GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");
    
    //set TEX to always refer to texture binding zero:
    glUseProgram(program); //bind program -- glUniform* calls refer to this program now
    
    glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0
    
    glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

// Generates a model and vao for a texture
GLuint gen_image(glm::vec2 pos, glm::vec2 size, float u1, float v1, float u2, float v2) {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    Load<TexProgram> &program = tex_program;
    glUseProgram(program->program);

    GLuint coord_buf = 0;
    glGenBuffers(1, &coord_buf);
    std::vector<glm::vec3> positions;

    positions.emplace_back(glm::vec3(pos.x, pos.y, 0.0f));
    positions.emplace_back(glm::vec3(pos.x + size.x, pos.y, 0.0f));
    positions.emplace_back(glm::vec3(pos.x + size.x, pos.y + size.y, 0.0f));
    positions.emplace_back(glm::vec3(pos.x, pos.y + size.y, 0.0f));

    glBindBuffer(GL_ARRAY_BUFFER, coord_buf);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * 3 * 4, positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(program->Position_vec4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(program->Position_vec4);

    GLuint tex_buf = 0;
    glGenBuffers(1, &tex_buf);
    std::vector<glm::vec2> tex_coords;

    tex_coords.emplace_back(glm::vec2(u1, v1));
    tex_coords.emplace_back(glm::vec2(u2, v1));
    tex_coords.emplace_back(glm::vec2(u2, v2));
    tex_coords.emplace_back(glm::vec2(u1, v2));

    glBindBuffer(GL_ARRAY_BUFFER, tex_buf);
    glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * 2 * 4, tex_coords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(program->TexCoord_vec2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(program->TexCoord_vec2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    GL_ERRORS();

    return vao;
}

void draw_image(GLuint image, GLuint tex, glm::vec4 color, float x, float y, float x_scale, float y_scale) {
    glUseProgram(tex_program->program);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(image);

    glUniformMatrix4fv(tex_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(glm::mat4(x_scale, 0.0f, 0.0f, 0.0f,  0.0f, y_scale, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  x, y, 0.0f, 1.0f)));
    glUniform4f(tex_program->COLOR_vec4, color.x, color.y, color.z, color.w);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    GL_ERRORS();

    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_ERRORS();
}

TexProgram::~TexProgram() {
    glDeleteProgram(program);
    program = 0;
}


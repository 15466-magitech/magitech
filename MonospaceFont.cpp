/*
 * Created by Russell Emerine on 10/30/23.
 * Mostly from https://learnopengl.com/In-Practice/Text-Rendering
 * Image drawing code from Matei Budiu (mateib)'s Auriga (game4)
 */

#include "gl_errors.hpp"

#include "MonospaceFont.hpp"
#include "TexProgram.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "load_save_png.hpp"

MonospaceFont::MonospaceFont(const std::string &filename) {
    glUseProgram(tex_program->program);
    
    glm::uvec2 size;
    std::vector<glm::u8vec4> data;
    load_png(data_path(filename), &size, &data, LowerLeftOrigin);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            size.x,
            size.y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data.data()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    GL_ERRORS();
    
    // yes, integer division here
    auto step = glm::uvec2(size.x / 32, size.y / 3);
    for (unsigned char cc = 0; cc < 128; cc++) {
        unsigned char c = cc;
        if (c < ' ') {
            c = ' ';
        }
        c -= ' ';
        
        auto start = glm::uvec2(
                step.x * (c % 32),
                step.y * (2 - c / 32)
        );
        std::vector<glm::vec2> tex_coords{
                glm::vec2(
                        ((float) start.x + 0.2 * (float) step.x) / (float) size.x,
                        (float) start.y / (float) size.y
                ),
                glm::vec2(
                        ((float) start.x + 0.8 * (float) step.x) / (float) size.x,
                        (float) start.y / (float) size.y
                ),
                glm::vec2(
                        ((float) start.x + 0.8 * (float) step.x) / (float) size.x,
                        ((float) start.y + (float) step.y) / (float) size.y
                ),
                glm::vec2(
                        ((float) start.x + 0.2 * (float) step.x) / (float) size.x,
                        ((float) start.y + (float) step.y) / (float) size.y
                )
        };
        
        // generate tex_buf
        GLuint tex_buf;
        glGenBuffers(1, &tex_buf);
        glBindBuffer(GL_ARRAY_BUFFER, tex_buf);
        glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * 2 * 4, tex_coords.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        GLuint vao;
        glGenVertexArrays(1, &vao);
        GLuint buf;
        glGenBuffers(1, &buf);
        
        GL_ERRORS();
        
        // now store character for later use
        Character character = {
                tex_buf,
                buf,
                vao,
        };
        char_info.emplace(cc, character);
    }
    
    glUseProgram(0);
    
    GL_ERRORS();
}

void MonospaceFont::draw(char c, glm::vec2 loc, glm::vec2 size) {
    // I currently recalculate the vertex array each time, but it could instead be precalculated upon
    // construction of the Terminal and stored there.
    
    // steps are multiplied by two to accound for the fact that it's [-1, 1] x [-1, 1]
    std::vector<glm::vec3> positions{
            glm::vec3(loc.x, loc.y, 0.0f),
            glm::vec3(loc.x + 2 * size.x, loc.y, 0.0f),
            glm::vec3(loc.x + 2 * size.x, loc.y + 2 * size.y, 0.0f),
            glm::vec3(loc.x, loc.y + 2 * size.y, 0.0f),
    };
    
    // The following code is from https://github.com/aehmttw/Auriga/blob/master/PlayMode.cpp
    glUseProgram(tex_program->program);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(char_info[c].vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, char_info[c].buf);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * 3 * 4, positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(tex_program->Position_vec4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(tex_program->Position_vec4);
    
    glBindBuffer(GL_ARRAY_BUFFER, char_info[c].tex_buf);
    glVertexAttribPointer(tex_program->TexCoord_vec2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(tex_program->TexCoord_vec2);
    
    glUniformMatrix4fv(tex_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(
            glm::mat4(
                    1, 0.0f, 0.0f, 0.0f,
                    0.0f, 1, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    loc.x, loc.y, 0.0f, 1.0f
            )));
    glUniform4f(tex_program->COLOR_vec4, 1, 1, 1, 1);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    
    GL_ERRORS();
}

//
// Created by Russell Emerine on 10/30/23.
//
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GL.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <glm/glm.hpp>
#include <map>

#include "data_path.hpp"
#include "read_write_chunk.hpp"
#include "Load.hpp"

/*
 * Represents a character's texture.
 * Taken from https://learnopengl.com/In-Practice/Text-Rendering,
 * but also i kinda changed everything because those features don't matter for monospace fonts.
 */
struct Character {
    GLuint tex_buf;
    // the following don't actually have information and are recalculated every use
    GLuint buf;
    GLuint vao;
};

struct MonospaceFont {
    GLuint texture; // ID handle of the atlas texture
    std::map<char, Character> char_info;
    
    explicit MonospaceFont(const std::string &filename);
    
    void draw(char c, glm::vec2 loc, glm::vec2 size);
};



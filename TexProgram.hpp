// From Matei Budiu (mateib)'s Auriga (game4)
#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

// Inspired by Jim McCann's message in the course Discord on how to create a textured quad:
// https://discord.com/channels/1144815260629479486/1154543452520984686/1156347836888256582

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct TexProgram {
    TexProgram();
	~TexProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;

	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
    GLuint COLOR_vec4 = -1U;

	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
};

extern Load<TexProgram> tex_program;

//For convenient scene-graph setup, copy this object:
// NOTE: by default, has texture bound to 1-pixel white texture -- so it's okay to use with vertex-color-only meshes.
extern Scene::Drawable::Pipeline tex_program_pipeline;

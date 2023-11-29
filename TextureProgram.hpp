#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, vertices tinted with vertex colors:
struct TextureProgram {
	TextureProgram();
	~TextureProgram();

	GLuint program = 0;
	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;
	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
};

extern Load< TextureProgram > texture_program;
extern Scene::Drawable::Pipeline texture_program_pipeline;
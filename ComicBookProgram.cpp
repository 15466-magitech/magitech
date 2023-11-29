#include "ComicBookProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include "glm/ext.hpp"

Scene::Drawable::Pipeline lit_color_texture_program_pipeline;

Load< ComicBookProgram > lit_color_texture_program(LoadTagEarly, []() -> ComicBookProgram const * {
	ComicBookProgram *ret = new ComicBookProgram();

	//----- build the pipeline template -----
	lit_color_texture_program_pipeline.program = ret->program;

	lit_color_texture_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	lit_color_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	lit_color_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;
    lit_color_texture_program_pipeline.SPECULAR_BRIGHTNESS_vec3 = ret->SPECULAR_BRIGHTNESS_vec3;
    lit_color_texture_program_pipeline.SPECULAR_SHININESS_float = ret->SPECULAR_SHININESS_float;
	lit_color_texture_program_pipeline.draw_frame = ret->draw_frame;
    lit_color_texture_program_pipeline.set_uniforms = [ret]() {
        glUniform1i(ret->LIGHT_TYPE_int, 1);
        glUniform3fv(ret->LIGHT_DIRECTION_vec3, 1,
                     glm::value_ptr(glm::normalize(glm::vec3(0.5f, 1.0f, -1.0f))));
        glUniform3fv(ret->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(0.6f, 0.65f, 0.7f)));
        glUniform3fv(ret->AMBIENT_LIGHT_ENERGY_vec3, 1,
                     glm::value_ptr(glm::vec3(0.5f, 0.45f, 0.4f)));
    };

	/* This will be used later if/when we build a light loop into the Scene:
	lit_color_texture_program_pipeline.LIGHT_TYPE_int = ret->LIGHT_TYPE_int;
	lit_color_texture_program_pipeline.LIGHT_LOCATION_vec3 = ret->LIGHT_LOCATION_vec3;
	lit_color_texture_program_pipeline.LIGHT_DIRECTION_vec3 = ret->LIGHT_DIRECTION_vec3;
	lit_color_texture_program_pipeline.LIGHT_ENERGY_vec3 = ret->LIGHT_ENERGY_vec3;
	lit_color_texture_program_pipeline.LIGHT_CUTOFF_float = ret->LIGHT_CUTOFF_float;
	*/

	//make a 1-pixel white texture to bind by default:
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	std::vector< glm::u8vec4 > tex_data(1, glm::u8vec4(0xff));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	lit_color_texture_program_pipeline.textures[0].texture = tex;
	lit_color_texture_program_pipeline.textures[0].target = GL_TEXTURE_2D;

	return ret;
});

ComicBookProgram::ComicBookProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"uniform mat4 OBJECT_TO_CLIP;\n"
		"uniform mat4x3 OBJECT_TO_LIGHT;\n"
		"uniform mat3 NORMAL_TO_LIGHT;\n"
        "uniform sampler2D DEPTH;\n"
        "uniform sampler2D DOT;\n"
        "uniform sampler2D SHADOW_DEPTH;\n"
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
		"}\n"
	,
		//fragment shader:
		"#version 330\n"
        "#define PI 3.1415926538\n"
        "uniform mat4 OBJECT_TO_CLIP;\n"
        "uniform sampler2D TEX;\n"
        "uniform sampler2D SHADOW_DEPTH;\n"
        "uniform int LIGHT_TYPE;\n"
		"uniform vec3 LIGHT_LOCATION;\n"
		"uniform vec3 LIGHT_DIRECTION;\n"
		"uniform vec3 LIGHT_ENERGY;\n"
        "uniform vec3 AMBIENT_LIGHT_ENERGY;\n"
        "uniform float SPECULAR_SHININESS;\n"
        "uniform vec3 SPECULAR_BRIGHTNESS;\n"
        "uniform float LIGHT_CUTOFF;\n"
        "uniform sampler2D DEPTH;\n"
        "uniform sampler2D DOT;\n"
        "uniform vec4 WINDOW_DIMENSIONS;\n"
        "uniform bool COMIC_BOOK;"
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
        "}\n"
        "float tex(float x, float y) {\n"
        "    return pow(1.0f - texture(DEPTH, vec2(x / WINDOW_DIMENSIONS.x, y / WINDOW_DIMENSIONS.y)).x, 0.1);\n"
        "}\n"
        "float outlineWeight(float x, float y) {\n"
        "    float a = 0.00025;\n"
        "    float b = 0.002;\n"
        "    float v = (abs(tex(x - 1, y) - tex(x, y)) + abs(tex(x, y - 1) - tex(x, y)));\n"
        "    return max(0.0, min(1.0, (v - a) / (b - a)));\n"
        "}\n"
        "float pixelShade(float x, float y, float shade) {\n"
        "   float bsize = 0.25;\n"
        "   int bracket = int(shade / bsize);\n"
        "   float frac = (shade - bracket * bsize) / bsize;\n"
        "   float res = 20.0;\n"
        "   float r = float((int(x) * 6833 + int(y) * 4643) % 7477) / 7477.0 / res;\n"
        "   float b = r + abs(x / res - float(int(x / res)) - 0.5) + abs(y / res - float(int(y / res)) - 0.5);\n"
        "   return (int(frac / 2.0 >= pow(b, 1) || frac / 2.0 >= pow(1 - b, 1)) + bracket) * bsize;\n"
        "}\n"
        "vec2 rotate(vec2 pos, float angle) {\n"
        "   return mat2(cos(angle), sin(angle), -sin(angle), cos(angle)) * pos;"
        "}\n"
        "vec3 pixelColor(vec2 pos, vec3 rgb) {\n"
        "   float res = 4.5;\n"
        "   vec3 cmy = vec3(1.0) - rgb;\n"
        "   vec2 posM = rotate(pos / res, 75.0 / 180.0 * PI);\n"
        "   vec2 posY = pos / res;\n"
        "   vec2 posC = rotate(pos / res, 15.0 / 180.0 * PI);\n"
        "   int c = int(texture(DOT, posC).r < cmy.x);\n"
        "   int m = int(texture(DOT, posM).r < cmy.y);\n"
        "   int y = int(texture(DOT, posY).r < cmy.z);\n"
        "   return vec3(1.0 - float(c), 1.0 - float(m), 1.0 - float(y));\n"
        "}\n"
        "int sampleShadow(vec3 position) {\n"
        "   return int(texture(SHADOW_DEPTH, vec2(position.x + 1.0, position.y + 1.0) / 2.0).x < -0.001 + (position.z + 1.0) / 2.0);\n"
        "}\n"
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
        "       //float b = float(pixelShade(gl_FragCoord.x, gl_FragCoord.y, (dot(n,-LIGHT_DIRECTION) * 0.5 + 0.5)));\n"
        "       float b = dot(n,LIGHT_DIRECTION) * 0.5 + 0.5;\n"
        "       //if (sampleShadow(position) > 0.0)\n"
        "       //    b = 0.0;\n"
		"		e = b * LIGHT_ENERGY + AMBIENT_LIGHT_ENERGY;\n"
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
        "   float specular = pow(max(dot(n, h), 0), SPECULAR_SHININESS);\n"
        "   e += specular * SPECULAR_BRIGHTNESS;\n"
		"	vec4 albedo = texture(TEX, texCoord) * color;\n"
		"   if(wireframe){\n"
		"   	fragColor = vec4(0.0,0.0,0.0,1.0);\n"
		"	}\n"
		"	else{\n"
		"		fragColor = vec4(e*albedo.rgb, 1.0);\n"
		"	}\n"
        "    \n"
        "   float weight = 0; \n"
        "   float kernel[] = float[25](0.0, 0.1, 0.2, 0.1, 0.0, 0.1, 0.4, 0.6, 0.4, 0.1, 0.2, 0.6, 1.0, 0.6, 0.2, 0.1, 0.4, 0.6, 0.4, 0.1, 0.0, 0.1, 0.2, 0.1, 0.0);\n"
        "   for (int i = 0; i < 5; i++) {\n"
        "       for (int j = 0; j < 5; j++) {\n"
        "           weight += kernel[i * 5 + j] * outlineWeight(gl_FragCoord.x + i - 2, gl_FragCoord.y + j - 2);\n"
        "       }\n"
        "   }\n"
        "   fragColor.xyz *= 1.0f - max(0.0f, weight);\n"
        "   float scale = max(WINDOW_DIMENSIONS.z / 1280.0, WINDOW_DIMENSIONS.w / 720.0);"
        "   fragColor.xyz = pixelColor(gl_FragCoord.xy / scale, fragColor.xyz);\n"
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

    WINDOW_DIMENSIONS = glGetUniformLocation(program, "WINDOW_DIMENSIONS");

	draw_frame = glGetUniformLocation(program,"wireframe");

    GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");
    GLuint DEPTH_sampler2D = glGetUniformLocation(program, "DEPTH");
    GLuint DOT_sampler2D = glGetUniformLocation(program, "DOT");
    GLuint SHADOW_DEPTH_sampler2D = glGetUniformLocation(program, "SHADOW_DEPTH");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0
    glUniform1i(DEPTH_sampler2D, 1); //set DEPTH to sample from GL_TEXTURE1
    glUniform1i(DOT_sampler2D, 2); //set DEPTH to sample from GL_TEXTURE2
    glUniform1i(SHADOW_DEPTH_sampler2D, 3); //set SHADOW_DEPTH to sample from GL_TEXTURE3

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

ComicBookProgram::~ComicBookProgram() {
	glDeleteProgram(program);
	program = 0;
}


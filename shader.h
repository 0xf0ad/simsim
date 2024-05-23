#ifndef SHADER_H
#define SHADER_H

#include "libs/glad/glad.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct{
	uint32_t ID;
} shader_t;

inline void compile_n_link(shader_t* shader, const char *vertexCode, const char *fragmentCode){

	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// compile the vertex shader and free it's string as long as we dont need it
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexCode, NULL);
	glCompileShader(vertex);
	// check for errors durring compiling the vertex shader
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertex, sizeof(infoLog), NULL, infoLog);
		fprintf(stderr, "ERROR : failed to compile the vertex shader : \n%s\n", infoLog);
	}

	// compile the fragment shader and free it's string as long as we dont need it
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentCode, NULL);
	glCompileShader(fragment);
	// check for errors durring compiling the fragment shader
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragment, sizeof(infoLog), NULL, infoLog);
		fprintf(stderr, "ERROR : failed to compile the fragment shader : \n%s\n", infoLog);
	}


	// 3 - attach Program
	// --------------------------------------------------------------
	// attach the shaders to the program ID
	shader->ID = glCreateProgram();
	glAttachShader(shader->ID, vertex);
	glAttachShader(shader->ID, fragment);
	glLinkProgram(shader->ID);

	// check linking errors
	glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(shader->ID, sizeof(infoLog), NULL, infoLog);
		fprintf(stderr, "ERROR : failed to link the shaders to the program : \n%s\n", infoLog);
	}

	// delete shader after linking them
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

inline void init_shader(shader_t* shader, const char* vertexPath, const char* fragmentPath){

	// 1 - retrieve the vertex and fragment source code from filepath
	// --------------------------------------------------------------
	char   *vertexCode,   *fragmentCode;
	FILE   *v_ShaderFile, *f_ShaderFile;
	size_t  v_StreamSize,  f_StreamSize;

	// open file
	v_ShaderFile = fopen(vertexPath  , "r");
	f_ShaderFile = fopen(fragmentPath, "r");

	// error handlling
	if(!(v_ShaderFile && f_ShaderFile)){
		if(!v_ShaderFile)
			fprintf(stderr, "could not open vertex shader file, check if it exists on %s\n", vertexPath);
		if(!f_ShaderFile)
			fprintf(stderr, "could not open fragment shader file, check if it exists on %s\n", fragmentPath);
		return;

	}else{

		// getting the size of the vertex shader file
		fseek(v_ShaderFile, 0L, SEEK_END);
		v_StreamSize = ftell(v_ShaderFile) + 1;
		rewind(v_ShaderFile);
		// getting the size of the fragment shader file
		fseek(f_ShaderFile, 0L, SEEK_END);
		f_StreamSize = ftell(f_ShaderFile) + 1;
		rewind(f_ShaderFile);

		// allocating the heap to be filed by the streams
		vertexCode   = (char*)malloc(v_StreamSize);
		fragmentCode = (char*)malloc(f_StreamSize);

		//read file's buffer content into streams
		unsigned int i = 0, j = 0;
		while(fgets((vertexCode + i), v_StreamSize, v_ShaderFile))
			i = ftell(v_ShaderFile);
		while(fgets((fragmentCode + j), f_StreamSize, f_ShaderFile))
			j = ftell(f_ShaderFile);

		// close files
		fclose(v_ShaderFile);
		fclose(f_ShaderFile);
	}

	compile_n_link(shader, vertexCode, fragmentCode);

	free(vertexCode);
	free(fragmentCode);
}


#if 0
void bind_ubo_to_shader(const uint32_t ID, ubo* uniformBuffer){
	GLuint index = glGetUniformBlockIndex(ID, uniformBuffer->name);
	glUniformBlockBinding(ID, index, uniformBuffer->bindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer->bindingPoint, uniformBuffer->UBO);
}
#endif

// activate the shader
// ------------------------------------------------------------------------
inline void use_shader(const uint32_t ID){
	glUseProgram(ID);
}


// utility uniform functions
// ------------------------------------------------------------------------

inline void set_uni_bool(const uint32_t ID, const char* name, bool value){

	GLint location = glGetUniformLocation(ID, name);
	glUniform1i(location, (int)value);
}

// ------------------------------------------------------------------------
inline void set_uni_int(const uint32_t ID, const char* name, int value){
	GLint location = glGetUniformLocation(ID, name);
	glUniform1i(location, value);
}

// ------------------------------------------------------------------------
inline void set_uni_float(const uint32_t ID, const char* name, float value){
	GLint location = glGetUniformLocation(ID, name);
	glUniform1f(location, value);
}

inline void set_uni_vec2(const uint32_t ID, const char* name, float x, float y){
	GLint location = glGetUniformLocation(ID, name);
	glUniform2f(location, x, y);
}

inline void set_uni_vec3(const uint32_t ID, const char* name, float x, float y, float z){
	GLint location = glGetUniformLocation(ID, name);
	glUniform3f(location, x, y, z);
}

inline void set_uni_vec4(const uint32_t ID, const char* name, float x, float y, float z, float w){
	GLint location = glGetUniformLocation(ID, name);
	glUniform4f(location, x, y, z, w);
}

#endif
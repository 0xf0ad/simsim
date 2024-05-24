#ifndef ED_H
#define ED_H

#include "shader.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <vector>

#include "libs/stbi/stb_image.h"

typedef struct{
	//position relative to node
	double pos[2];
	double value;
	bool selected;
} pin_t;

typedef struct{
	double value;
	pin_t* in;
	pin_t* out;
} link_t;

typedef struct{
	void* texID;
	double dims[2];
	double pos[2];
} Quad_t;

typedef struct{
	uint64_t id;
	Quad_t quad;
	bool selected;
	std::vector<pin_t> in_pins;
	std::vector<pin_t> out_pins;
	// the tarnsfer function of the node
	// take pointers of i/o pin and modify the output pins  
	void (*H)(std::vector<pin_t>&, std::vector<pin_t>&);
} node_t;

typedef struct{
	double scale;
	double step;
	double offset[2];
} grid_t;

// you can think of it as a sceen struct
typedef struct{
	double aspectratio;
	uint64_t nodescount;
	uint16_t resol[2];
	grid_t grid;
	pin_t* connector;
	std::vector<node_t> nodes;
	std::vector<link_t> links;
} editor_t;

inline void drawnodes(const editor_t* edtr){
	for(uint64_t i = 0; i < edtr->nodescount; i++){
		//TODO
	}
};

inline bool mouseovernode(double x, double y, const node_t* node){
	//TODO
	return (x >= (node->quad.pos[0] + node->quad.dims[0]) && 
	        x <= (node->quad.pos[0] - node->quad.dims[0]) &&
	        y >= (node->quad.pos[1] + node->quad.dims[1]) &&
	        y <= (node->quad.pos[1] - node->quad.dims[1]));
};

inline GLuint load_texture(const char* path, int* w, int* h){

	// apparently ImGui drawlist cant draw two textures if they are bind to the same slot
	// so this opptimization wont do

	//static std::unordered_map<std::string, GLuint> loaded_textures;

	//if(loaded_textures.find(path) != loaded_textures.end())
	//	return loaded_textures[path];


	stbi_set_flip_vertically_on_load(true);
	int nrChannels;
	unsigned char* data = stbi_load(path, w, h, &nrChannels, 0);

	if(!data){
		fprintf(stderr, "failed to load texture\n");
		return -1;
	}

	GLenum format = GL_FALSE;
	GLenum encodings[] = {GL_FALSE, GL_RED, GL_FALSE, GL_RGB, GL_RGBA};
	format = encodings[nrChannels];

	if(!format)
		fprintf(stderr, "texture: \"%s\" has %d component which is invalid\n", path, nrChannels);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexImage2D(GL_TEXTURE_2D, 0, format, *w, *h, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//loaded_textures[path] = textureID;
	stbi_image_free(data);

	return textureID;
}

inline void process(editor_t* p_editor){
	for(size_t i = 0; i < p_editor->nodescount; i++){
		node_t* node = &p_editor->nodes[i];
		node->H(node->in_pins, node->out_pins);
	}

	for(size_t i = 0; i < p_editor->links.size(); i++){
		double value = p_editor->links[i].in->value;
		p_editor->links[i].out->value = value;
		p_editor->links[i].value = value;
	}
	
}

void buffer_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = in[0].value;
}

void not_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = !in[0].value;
}

void and_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = in[0].value && in[1].value;
}

void or_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = in[0].value || in[1].value;
}

void nand_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = !(in[0].value && in[1].value);
}

void nor_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = !(in[0].value || in[1].value);
}

void xor_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = in[0].value == in[1].value ? 1.l : 0.l;
}

void xnor_transfer(std::vector<pin_t>& in, std::vector<pin_t>& out){
	out[0].value = in[0].value == in[1].value ? 0.l : 1.l;
}

#endif

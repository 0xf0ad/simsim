#include "shader.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <vector>

#include "libs/stbi/stb_image.h"

typedef struct{
	//position relative to node
	float pos[2];
	bool selected;
} pin_t;

typedef struct{
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
	pin_t* in_pins;
	pin_t* out_pins;
	Quad_t quad;
	bool selected;
	std::vector<pin_t>  pins;
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
	std::vector<node_t> nodes;
	std::vector<link_t> links;
} editor_t;

inline void loop(const editor_t* edtr){
//TODO
};

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

inline void show_node_menu(node_t* node){
	//TODO
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

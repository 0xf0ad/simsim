#ifndef ED_H
#define ED_H

#include <cstdint>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "components.h"
#include "matrix.h"

#include "libs/stbi/stb_image.h"
#include "libs/glad/glad.h"

typedef struct{
	double scale;
	double step;
	double offset[2];
} grid_t;

// you can think of it as a sceen struct
typedef struct{
	double aspectratio;
	uint64_t num_indp_vol_sources;
	uint16_t resol[2];
	grid_t grid;
	node_t* head;
	pin_t* connector;
	std::vector<component_t> components;
	std::vector<pin_t>  pins;
} editor_t;


inline bool mouse_over_quad(double x, double y, const Quad_t* quad){
	//TODO
	return (x >= (quad->pos[0] + quad->dims[0]) && 
	        x <= (quad->pos[0] - quad->dims[0]) &&
	        y >= (quad->pos[1] + quad->dims[1]) &&
	        y <= (quad->pos[1] - quad->dims[1]));
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
		fprintf(stderr, "failed to load texture: %s\n", path);
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

inline node_t* add_node(node_t* head){
	node_t* node = head;
	uint64_t id = 0;

	if(node){
		while(node->next)
			id++, node = node->next;

		node->next = (node_t*) malloc(sizeof(node_t));
		node = node->next;
		node->prev = node;
	} else {
		node = (node_t*) malloc(sizeof(node_t));
		node->prev = NULL;

	}
	node->next = NULL;
	node->id = id;
	return node;
}

inline void update_ids(node_t* node){
	uint64_t id = node->id;
	while((node = node->next))
		node->id = ++id;
}

inline void rm_node(editor_t* editor,node_t* node){
	if(editor){
		for(uint64_t i = 0; i < editor->components.size(); i++){
			if(editor->components[i].from.connected_node == node)
				editor->components[i].from.connected_node = NULL;
			if(editor->components[i].to.connected_node == node)
				editor->components[i].to.connected_node = NULL;
		}
	}

	if(node->prev)
		node->prev->next = node->next;

	update_ids(node->prev);

	free(node);
}

// merge node1 into node0 and replace node1's occurences to node0 if provided editor
inline node_t* merge_nodes(editor_t* editor, node_t* node0, node_t* node1){
	if(editor){
		for(uint64_t i = 0; i < editor->components.size(); i++){
			if(editor->components[i].from.connected_node == node1)
				editor->components[i].from.connected_node = node0;
			if(editor->components[i].to.connected_node == node1)
				editor->components[i].to.connected_node = node0;
		}
	}

	if(node1->prev)
		node1->prev->next = node1->next;

	free(node1);
}

inline uint64_t get_num_nodes(editor_t* p_editor){
	uint64_t count = 1;
	node_t* node = p_editor->head;
	while((node = node->next))
		count++;
	return count;
}

inline uint64_t get_num_indep_volt_src(editor_t* p_editor){
	//TODO
}

inline void* constract_matrices(editor_t* p_editor){
	uint64_t n = get_num_nodes(p_editor);
	uint64_t m = get_num_indep_volt_src(p_editor);
	matrix_t A, C;
	init_matrix(&A, n+m, n+m);
	init_matrix(&C, n+m, n+m);
	fill_zeros(&A); fill_zeros(&C);
	for (uint64_t crnt_comp = 0; crnt_comp < p_editor->components.size(); crnt_comp++){
		component_t* comp = &p_editor->components[crnt_comp];
		uint64_t i = comp->from.connected_node->id;
		uint64_t j = comp->to.connected_node->id;
		switch(comp->defenition.type){
			case resestor:{
				double G = 1./comp->caracteristic;
				A.entries[i][j] += G;
				A.entries[i][i] -= G;
				A.entries[j][j] -= G;
				A.entries[j][i] += G;
				break;
			}
			case capacitor:{
				double Cap = comp->caracteristic;
				C.entries[i][j] += Cap;
				C.entries[i][i] -= Cap;
				C.entries[j][j] -= Cap;
				C.entries[j][i] += Cap;
				break;
			}
			case inductor:{
				uint64_t k = n+comp->id;
				double L = comp->caracteristic;
				A.entries[i][k] += 1.l;
				A.entries[j][k] -= 1.l;
				A.entries[k][i] += 1.l;
				A.entries[k][j] -= 1.l;
				C.entries[k][k] -= L;
				break;
			}
			default:
				break;
		}
	}
}

inline void process(editor_t* p_editor){

	//matrix_t G = constract_admitance_matrix(p_editor);
	//matrix_t B = constract_B_matrix();
	//free_mat(&G);
}


#endif

#ifndef ED_H
#define ED_H

#include <ginac/basic.h>
#include <ginac/matrix.h>
#include <ginac/symbol.h>
#include <iostream>
#include <istream>
#include <math.h>
#include <ostream>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "components.h"

#include "stbi/stb_image.h"
#include "glad/glad.h"

#include <ginac/ginac.h>
#include <string>
#include <vector>

typedef struct{
	double scale;
	double step;
	double offset[2];
} grid_t;

enum em_shape_type {
    EM_SHAPE_RECT,
    EM_SHAPE_CIRCLE,
    EM_SHAPE_POLYGON,
};

enum em_source_type {
    EM_SOURCE_POINT,      // Point source (omnidirectional)
    EM_SOURCE_LINE,       // Line source
    EM_SOURCE_PLANE,      // Plane wave
};

enum em_probe_type {
    EM_PROBE_E_FIELD,     // Electric field probe
    EM_PROBE_H_FIELD,     // Magnetic field probe
    EM_PROBE_POWER,       // Power/Poynting vector probe
};

// EM wave source
struct em_source_t {
    em_source_type type;
    ImVec2 pos;           // Position (canvas coords)
    float frequency;      // Hz
    float amplitude;      // V/m or A/m
    float phase;          // radians
    float angle;          // direction for plane wave (radians)
    bool active;
    char name[32];
};

// EM field probe
struct em_probe_t {
    em_probe_type type;
    ImVec2 pos;           // Position (canvas coords)
    float value;          // Current measured value
    bool active;
    char name[32];
};

// FDTD simulation grid
struct fdtd_grid_t {
    int nx, ny;           // Grid dimensions
    float dx, dy;         // Cell size (meters)
    float dt;             // Time step (seconds)
    int time_step;        // Current time step
    
    // Field arrays (2D Yee grid for TM mode: Ez, Hx, Hy)
    std::vector<float> Ez;  // Electric field (z-component)
    std::vector<float> Hx;  // Magnetic field (x-component)
    std::vector<float> Hy;  // Magnetic field (y-component)
    
    // Material property grids
    std::vector<float> eps;   // Permittivity at each cell
    std::vector<float> mu;    // Permeability at each cell
    std::vector<float> sigma; // Conductivity at each cell
    
    bool initialized;
    bool running;
    float max_field;      // For visualization scaling
};

// A single drawn shape
struct em_shape_t {
    em_shape_type type;
    em_material_t material;

    // RECT: p0 = top-left corner, p1 = bottom-right corner (canvas coords)
    // CIRCLE: p0 = centre, radius stored in p1.x
    // POLYGON: vertices stored in poly_pts
    ImVec2 p0, p1;
    std::vector<ImVec2> poly_pts;   // polygon vertices (canvas coords)

    bool selected;
};


// you can think of it as a sceen struct
typedef struct{
	double aspectratio;
	uint64_t num_indp_vol_sources;
	uint16_t resol[2];
	grid_t grid;
	// we consider the head as the ground point
	node_t* head;
	pin_t* connector;
	std::vector<component_t> components;
	std::vector<link_t> links;
	std::vector<pin_t*> pins;
	
	std::vector<component_t*> selected_components;
	std::vector<link_t*> selected_links;

	// EM simulation data
	std::vector<em_shape_t> shapes;
	std::vector<em_source_t> sources;
	std::vector<em_probe_t> probes;
	em_material_t active_material;
	fdtd_grid_t fdtd;

	// drawing state
	em_shape_type draw_mode;   // what we are about to draw
	bool          drawing;     // mid-draw
	ImVec2        draw_start;  // first click (canvas coords)
	std::vector<ImVec2> poly_wip; // polygon work-in-progress vertices

	// selection / interaction
	int  selected_idx;         // -1 = none (shape)
	int  selected_source_idx;  // -1 = none
	int  selected_probe_idx;   // -1 = none
	
	bool panning;
	ImVec2 pan_last;

	// shape editing
	bool  editing_shape;
	int   edit_handle;         // which handle is being dragged (-1 = none, 0-7 = corners/edges)
} editor_t;


inline bool mouse_over_quad(editor_t* editor, double x, double y, const Quad_t* quad){
	bool result = false;
	if (x <= (editor->grid.offset[0] + ((quad->pos[0] + (quad->dims[0] / 4.l)) * editor->grid.scale)))
	if (x >= (editor->grid.offset[0] + ((quad->pos[0] - (quad->dims[0] / 4.l)) * editor->grid.scale)))
	if (y >= (editor->grid.offset[1] + ((quad->pos[1] - (quad->dims[1] / 4.l)) * editor->grid.scale)))
	if (y <= (editor->grid.offset[1] + ((quad->pos[1] + (quad->dims[1] / 4.l)) * editor->grid.scale)))
		result = true;
	return result;
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

#ifndef LIGHTTHEME
	for(int i = 0; i < *w * *h; i++){
		unsigned char* pixel = data + (i * nrChannels);
		for(int j = 0; j < nrChannels-1; j++)
			pixel[j] = 255 - pixel[j];
	}
#endif

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

inline uint64_t get_num_elements(editor_t* editor, component_type type){
	uint64_t count = 0;
	for(auto i : editor->components)
		count += (i.definition.type == type);

	// accumudate for unlisted inductors if they where coupled
	if(type == inductor)
		count += 2 * get_num_elements(editor, coupled_inductors);

	return count;
}

inline node_t* add_node(node_t* head){
	node_t* node = head;
	uint64_t id;

	if(node){
		id = node->id + 1;

		while(node->next)
			id++, node = node->next;

		node->next = (node_t*) malloc(sizeof(node_t));
		node->next->prev = node;
		node = node->next;
	} else {
		id = 0;
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
	if(editor)
		for(uint64_t i = 0; i < editor->components.size(); i++)
			for(uint8_t j = 0; j < editor->components[i].definition.num_pins; j++)
				if(editor->components[i].pins[j].connected_node == node)
					editor->components[i].pins[j].connected_node = NULL;

	if(node->prev)
		node->prev->next = node->next;
	if(node->next)
		node->next->prev = node->prev;

	update_ids(node->prev);

	free(node);
}

// merge node1 into node0 and replace node1's occurences to node0 if provided editor
inline node_t* merge_nodes(editor_t* editor, node_t* node0, node_t* node1){
	if(node0 == node1)
		return node0;
	if(node1 == editor->head){
		node1 = node0;
		node0 = editor->head;
	}

	if(editor)
		for(uint64_t i = 0; i < editor->components.size(); i++)
			for(uint8_t j = 0; j < editor->components[i].definition.num_pins; j++)
				if(editor->components[i].pins[j].connected_node == node1)
					editor->components[i].pins[j].connected_node = node0;

	if(node1->prev)
		node1->prev->next = node1->next;
	if(node1->next)
		node1->next->prev = node1->prev;

	free(node1);
	update_ids(node0);
	return node0;
}

inline uint64_t get_num_nodes(editor_t* p_editor){
	uint64_t count = 1; // by counting this one itself
	node_t* node = p_editor->head;
	while((node = node->next))
		count++;
	return count;
}

inline uint64_t get_num_indep_volt_src(editor_t* p_editor){
	uint64_t count = 0;
	for(auto comp : p_editor->components)
		switch (comp.definition.type){
			case inductor:
			case indp_voltage_source:
			case volt_cont_volt_source:
			case curr_cont_volt_source:
			case curr_cont_curr_source:
			case operational_amplifier:
				count += comp.pins[0].connected_node && comp.pins[1].connected_node;
			default:
				break;
		}
	return count;
}

enum computationtype{
	symbolic,
	numerical
};

inline void constract_matrices(editor_t* p_editor, GiNaC::matrix* pA, GiNaC::matrix* px, GiNaC::matrix* pZ, GiNaC::symbol s, computationtype ct){
	uint64_t n = get_num_nodes(p_editor);
	uint64_t m = get_num_indep_volt_src(p_editor);
	uint64_t source_count = 0;
	// components of A matrix
	GiNaC::matrix G(n, n), B(n, m), C(m, n), D(m, m);
	// vectors of unknowns
	GiNaC::matrix V(n, 1), J(m, 1);
	// results
	GiNaC::matrix I(n, 1), Ev(m, 1);
	for (uint64_t crnt_comp = 0; crnt_comp < p_editor->components.size(); crnt_comp++){
		component_t* comp = &p_editor->components[crnt_comp];
		if(comp->definition.type == ground) continue;
		if(comp->pins[0].connected_node && comp->pins[1].connected_node){
			uint64_t i = comp->pins[0].connected_node->id;
			uint64_t j = comp->pins[1].connected_node->id;
			switch(comp->definition.type){
				case undefined:{
					assert(1);
				}
				case resistor:{
					GiNaC::symbol R(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					double numR = comp->caracteristic;
					G(i, i) += (ct == symbolic) ? 1/R : 1/numR;
					G(i, j) -= (ct == symbolic) ? 1/R : 1/numR;
					G(j, i) -= (ct == symbolic) ? 1/R : 1/numR;
					G(j, j) += (ct == symbolic) ? 1/R : 1/numR;
					break;
				}
				case capacitor:{
					GiNaC::symbol Cap(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					double numC = comp->caracteristic;
					G(i, i) += (ct == symbolic) ? s * Cap : s * numC;
					G(i, j) -= (ct == symbolic) ? s * Cap : s * numC;
					G(j, i) -= (ct == symbolic) ? s * Cap : s * numC;
					G(j, j) += (ct == symbolic) ? s * Cap : s * numC;
					break;
				}
				case inductor:{
					GiNaC::symbol L(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					B(i, source_count) =  1;
					B(j, source_count) = -1;
					C(source_count, i) =  1;
					C(source_count, j) = -1;
					D(source_count, source_count) -= ct == symbolic ? s * L : s * comp->caracteristic;
					J(source_count, 0) += GiNaC::symbol("I_" + L.get_name());
					source_count++;
					break;
				}
				case coupled_inductors:{
					uint64_t li = comp->L1->id;
					uint64_t lj = comp->L2->id;
					GiNaC::symbol M(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					double numM = comp->caracteristic * sqrt(comp->L1->caracteristic * comp->L2->caracteristic);
					D(li, lj) -= (ct == symbolic) ? s * M : s * numM;
					D(lj, li) -= (ct == symbolic) ? s * M : s * numM;
					break;
				}
				case indp_voltage_source:{
					GiNaC::symbol E(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					B(i, source_count) =  1;
					B(j, source_count) = -1;
					C(source_count, i) =  1;
					C(source_count, j) = -1;
					J(source_count, 0) += GiNaC::symbol("I_" + E.get_name());
					Ev(source_count, 0) = (ct == symbolic) ? E : (GiNaC::ex)comp->caracteristic;
					source_count++;
					break;
				}
				case indp_current_source:{
					GiNaC::symbol Intens(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					double numI = comp->caracteristic;
					I(i, 0) += ct == symbolic ? Intens : (GiNaC::ex)numI;
					I(j, 0) -= ct == symbolic ? Intens : (GiNaC::ex)numI;
					break;
				}
				case volt_cont_volt_source:{
					uint64_t ci = comp->pins[2].connected_node->id;
					uint64_t cj = comp->pins[3].connected_node->id;
					GiNaC::symbol element(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					double numelement = comp->caracteristic;
					B(i, source_count) =  1;
					B(j, source_count) = -1;
					C(source_count, i) =  1;
					C(source_count, j) = -1;
					C(source_count, ci) = ct == symbolic ? -element : (GiNaC::ex)-numelement;
					C(source_count, cj) = ct == symbolic ?  element : (GiNaC::ex) numelement;
					J(source_count, 0) = GiNaC::symbol("I_" + element.get_name());
					source_count++;
					break;
				}
				case volt_cont_curr_source:{
					uint64_t ci = comp->pins[2].connected_node->id;
					uint64_t cj = comp->pins[3].connected_node->id;
					GiNaC::symbol idk(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					double numidk = comp->caracteristic;
					G(i, ci) += ct == symbolic ? idk : (GiNaC::ex)numidk;
					G(i, cj) -= ct == symbolic ? idk : (GiNaC::ex)numidk;
					G(j, ci) -= ct == symbolic ? idk : (GiNaC::ex)numidk;
					G(j, cj) += ct == symbolic ? idk : (GiNaC::ex)numidk;
					break;
				}
				case curr_cont_volt_source:{
					GiNaC::symbol idk(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					B(i, source_count) =  1;
					B(j, source_count) = -1;
					C(source_count, i) =  1;
					C(source_count, j) = -1;
					D(source_count, comp->Vcont->id) -= (ct == symbolic) ? idk : (GiNaC::ex)comp->caracteristic;
					J(source_count, 0) = GiNaC::symbol("I_" + idk.get_name());
					source_count++;
					break;
				}
				case curr_cont_curr_source:{
					GiNaC::symbol idk(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					B(i, source_count) =  1;
					B(j, source_count) = -1;
					D(source_count, comp->Vcont->id) -= ct == symbolic ? idk : (GiNaC::ex)comp->caracteristic;
					D(source_count, source_count) = 1;
					J(source_count, 0) = GiNaC::symbol("I_" + idk.get_name());
					source_count++;
					break;
				}
				case operational_amplifier:{
					uint64_t ci = comp->pins[2].connected_node->id;
					GiNaC::symbol idk(std::string(comp->definition.abriv) + "_" + std::to_string(comp->id));
					// ci = Vout (which is a votage source)
					B(ci, source_count) =  1;
					C(source_count, i)  =  1;
					C(source_count, j)  = -1;
					J(source_count, 0) = GiNaC::symbol("I_" + idk.get_name());
					source_count++;
					break;
				}
				case graph:
				case ground:
					break;
			}
		}
	}

	for(uint64_t i = 0; i < n; i++)
		V(i, 0) = GiNaC::symbol("V_" + std::to_string(i));


	// I cant find better way to concatinate matrices
	// A = [G B] 
	//     [C D]
	// start from i = 1 because 0 is the ground node
	GiNaC::matrix A(n+m-1, n+m-1);
	for(uint64_t i = 1; i < n+m; i++)
		for(uint64_t j = 1; j < n+m; j++)
			if(i < n && j < n)
				A(i-1, j-1) = G(i, j);
			else if(i < n && j >= n)
				A(i-1, j-1) = B(i, j - n);
			else if(i >= n && j < n)
				A(i-1, j-1) = C(i - n, j);
			else if(i >= n && j >= n)
				A(i-1, j-1) = D(i - n, j - n);


	// x = [V]
	//     [J]
	GiNaC::matrix x(n+m-1, 1);
	for(uint64_t i = 1; i < n+m; i++)
		if(i < n)
			x(i-1, 0) = V(i, 0);
		else
			x(i-1, 0) = J(i - n, 0);


	// Z = [I ]
	//     [Ev]
	GiNaC::matrix Z(n+m-1, 1);
	for(uint64_t i = 1; i < n+m; i++)
		if(i < n)
			Z(i-1, 0) = I(i, 0);
		else
			Z(i-1, 0) = Ev(i - n, 0);

	*pA = A;
	*px = x;
	*pZ = Z;
}

#endif

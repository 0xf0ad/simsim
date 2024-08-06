#ifndef LOGICCOMP_H_
#define LOGICCOMP_H_

#include <stdint.h>
#include <vector>

typedef struct{
	void* texID;
	double dims[2];
	double pos[2];
} Quad_t;

enum component_type{
	// TODO: expand
	wire,
	voltage_source,
	current_source,
	resestor,
	capacitor,
	inductor,
	diode,
	transistor
};

struct node_t{
	node_t* next;
	node_t* prev;
	uint64_t id;
};

// so you dont have to write struct every time 
typedef struct node_t node_t;

typedef struct{
	//position relative to node
	double pos[2];
	node_t* connected_node;
	bool selected;
} pin_t;

typedef struct{
	component_type type;
	const char* name;
	const char* abriv;
	// each element correspend to the number
	// of pins on each side of the component
	// the order is: <, A, V, >
} component_def_t;

typedef struct{
	uint64_t id;
	component_def_t defenition;
	// this could be the resistance for R, the capacitance for C,
	// the inductance for L.
	double caracteristic;
	// represent the node that are connected to anode and cathode
	pin_t from;
	pin_t to;
	// rendeing stuff
	Quad_t quad;
} component_t;

typedef struct{
	pin_t pins[2];
} link_t;


component_def_t components[] = {
	{wire,           "",               ""},
	{voltage_source, "voltage source", "Vs"},
	{current_source, "current source", "Is"},
	{resestor,       "resestor",       "R"},
	{capacitor,      "capacitor",      "C"},
	{inductor,       "inductor",       "L"},
	{diode,          "diode",          "D"},
	{transistor,     "transistor",     "T"},
};

#endif //LOGICCOMMP_H_

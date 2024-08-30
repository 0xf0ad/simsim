#ifndef LOGICCOMP_H_
#define LOGICCOMP_H_

#include <stdint.h>
//#include <vector>

typedef struct{
	void* texID;
	float rot;
	double dims[2];
	double pos[2];
} Quad_t;

enum component_type{
	// TODO: expand
	undefined,
	ground,
	resistor,
	inductor,
	capacitor,
	coupled_inductors,
	indp_voltage_source,
	indp_current_source,
	volt_cont_volt_source,
	volt_cont_curr_source,
	curr_cont_volt_source,
	curr_cont_curr_source,
	operational_amplifier,
	graph
};

struct node_t{
	struct node_t* next;
	struct node_t* prev;
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
	uint8_t num_pins;
	const char* name;
	const char* abriv;
} component_def_t;

struct component_t{
	uint64_t id;
	component_def_t definition;
	// this could be the resistance for R, the capacitance for C,
	// the inductance for L, gain for sources
	double caracteristic;
	// represent the node that are connected to anode and cathode
	// TODO: make it a pin_t[]
	//pin_t n, p;
	// these are reserved to dependent votage and curren sources
	//pin_t cn, cp;
	pin_t *pins;
	//TODO: make em component_t[]
	// this becume important with current controlled sources
	union{
		// for F and H components
		struct{
			component_t* Vcont;
			component_t* shouldntbeused;
		};
		// for coupled inductors
		struct{
			component_t* L1;
			component_t* L2;
		};
	};
	//these are relevent in the case of coupled inductors
	// rendering stuff
	Quad_t quad;
};

typedef struct component_t component_t;

typedef struct{
	pin_t* pins[2];
	//std::vector<double[2]> joints;
} link_t;


component_def_t components[] = {
	{undefined,             0, "",                                  ""},
	{ground,                1, "ground",                            "GND"},
	{resistor,              2, "resistor",                          "R"},
	{inductor,              2, "inductor",                          "L"},
	{capacitor,             2, "capacitor",                         "C"},
	{coupled_inductors,     4, "coupled inductors",                 "K"},
	{indp_voltage_source,   2, "voltage source",                    "E"},
	{indp_current_source,   2, "current source",                    "I"},
	{volt_cont_volt_source, 4, "voltage controlled voltage source", "VCVS"},
	{volt_cont_curr_source, 4, "voltage controlled current source", "VCCS"},
	{curr_cont_volt_source, 2, "current controlled voltage source", "CCVS"},
	{curr_cont_curr_source, 2, "current controlled current source", "CCCS" /*soviet moment*/ },
	{operational_amplifier, 3, "operational amplifier",             "OpAmp"},
	{graph,                 2, "graph",                             "graph"}
};

#endif //LOGICCOMMP_H_

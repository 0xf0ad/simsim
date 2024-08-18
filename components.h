#ifndef LOGICCOMP_H_
#define LOGICCOMP_H_

#include <stdint.h>

typedef struct{
	void* texID;
	double rotation;
	double dims[2];
	double pos[2];
} Quad_t;

enum component_type{
	// TODO: expand
	undefined,
	resestor,
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
	const char* name;
	const char* abriv;
} component_def_t;

struct component_t{
	uint64_t id;
	component_def_t defenition;
	// this could be the resistance for R, the capacitance for C,
	// the inductance for L, gain for sources
	double caracteristic;
	// represent the node that are connected to anode and cathode
	// TODO: make it a pin_t[]
	pin_t n, p;
	// these are reserved to dependent votage and curren sources
	pin_t cn, cp;
	//TODO: make em component_t[]
	// this becume important with current controlled sources
	component_t* Vcontroll;
	//these are relevent in the case of coupled inductors
	component_t* L1;
	component_t* L2;
	// rendering stuff
	Quad_t quad;
};

typedef struct component_t component_t;

typedef struct{
	pin_t* pins[2];
} link_t;


component_def_t components[] = {
	{undefined,             "",                                  ""},
	{resestor,              "resestor",                          "R"},
	{inductor,              "inductor",                          "L"},
	{capacitor,             "capacitor",                         "C"},
	{coupled_inductors,     "coupled inductors",                 "K"},
	{indp_voltage_source,   "voltage source",                    "V"},
	{indp_current_source,   "current source",                    "I"},
	{volt_cont_volt_source, "voltage controlled voltage source", "VCVS"},
	{volt_cont_curr_source, "voltage controlled current source", "VCCS"},
	{curr_cont_volt_source, "current controlled voltage source", "CCVS"},
	{curr_cont_curr_source, "current controlled current source", "CCCS" /*soviet moment*/ },
	{operational_amplifier, "operational amplifier",             "OpAmp"}
};

#endif //LOGICCOMMP_H_

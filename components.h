#ifndef LOGICCOMP_H_
#define LOGICCOMP_H_

#include <stdint.h>
#include <vector>

enum logicgatetype_t {
    undeffined,
    buffergate,
    notgate,
    andgate,
    nandgate,
    orgate,
    norgate,
    xorgate,
    xnorgate,
	logicprob,
	logicstate
};

enum pin_types{
	input,
	output
};

typedef double signal_t;

typedef struct{
	//position relative to node
	double pos[2];
	signal_t value;
	// does it receive signal or transmet it
	bool type;
	bool selected;
} pin_t;

typedef struct{
	signal_t value;
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
	void (*H)(pin_t*, pin_t*);
} node_t;

typedef struct{
    logicgatetype_t type;
    const char* name;
    uint8_t numinputs;
    uint8_t numoutputs;
    void (*H)(pin_t*, pin_t*);
} logicgate_t;


// ======= in logic we only manipulate and care about current ========

void buffer_transfer(pin_t* in, pin_t* out){
	out[0].value = in[0].value;
}

void not_transfer(pin_t* in, pin_t* out){
	out[0].value = !in[0].value;
}

void and_transfer(pin_t* in, pin_t* out){
	out[0].value = in[0].value && in[1].value;
}

void or_transfer(pin_t* in, pin_t* out){
	out[0].value = in[0].value || in[1].value;
}

void nand_transfer(pin_t* in, pin_t* out){
	out[0].value = !(in[0].value && in[1].value);
}

void nor_transfer(pin_t* in, pin_t* out){
	out[0].value = !(in[0].value || in[1].value);
}

void xor_transfer(pin_t* in, pin_t* out){
	out[0].value = in[0].value == in[1].value ? 1.l : 0.l;
}

void xnor_transfer(pin_t* in, pin_t* out){
	out[0].value = in[0].value == in[1].value ? 0.l : 1.l;
}

// these doesnt transfer
void logicprob_transfer(pin_t* in, pin_t* out){}
void logicstate_transfer(pin_t* in, pin_t* out){}


logicgate_t nodes[] = {
    {undeffined, "undeffined",  0, 0, NULL},
    {buffergate, "buffer",      1, 1, &buffer_transfer},
    {notgate,    "not",         1, 1, &not_transfer},
    {andgate,    "and",         2, 1, &and_transfer},
    {nandgate,   "nand",        2, 1, &nand_transfer},
    {orgate,     "or",          2, 1, &or_transfer},
    {norgate,    "nor",         2, 1, &nor_transfer},
    {xorgate,    "xor",         2, 1, &xor_transfer},
    {xnorgate,   "xnor",        2, 1, &xnor_transfer},
    {logicprob,  "logic prob",  0, 1, &logicprob_transfer},
    {logicstate, "logic state", 1, 0, &logicstate_transfer}
};

#endif //LOGICCOMMP_H_

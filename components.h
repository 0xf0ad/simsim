#ifndef LOGICCOMP_H_
#define LOGICCOMP_H_

#include <stdint.h>

enum logicgatetype_t {
    undeffined,
    buffergate,
    notgate,
    andgate,
    nandgate,
    orgate,
    norgate,
    xorgate,
    xnorgate
};

typedef struct{
    logicgatetype_t type;
    uint8_t numinputs;
    uint8_t numoutputs;
} logicgate_t;

#endif //LOGICCOMMP_H_

#ifndef __MAIN_H
#define __MAIN_H

#include <gsl/gsl_rng.h>

typedef float pairType;
typedef struct PairStruct {
    pairType item1;
    pairType item2;
} pair;

extern gsl_rng *shared_r;

#endif
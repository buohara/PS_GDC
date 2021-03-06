/*
frontmatter
*/
#ifndef DCSIMSEP_H
#define DCSIMSEP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <suitesparse/cs.h>
#include <suitesparse/klu.h>
#include "ps.h"

#define MAX_T 1800.0
#define BIG_EPS 1e-6
#define SMALL_EPS 1e-12

#ifdef __cplusplus
extern "C"{
#endif

/*
build Bbus matrices.
*/
cs_di** get_bbus(PS *ps);

/*
solve DC load flow
*/
int dcpf(PS *ps);

/*
run cascading failure simulation
*/
int dcsimsep(PS *ps,Outage *outages);

/*
redispatch generators
*/
int redispatch(PS *ps);

/*
check relays for overcurrent and trip branches as needed
*/
double update_relays(PS *ps,Outage *outages,double t);

#ifdef __cplusplus
}
#endif

#endif

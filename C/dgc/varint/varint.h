#ifndef VARINT_H
#define VARINT_H

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "trackball.h"
#include "../simplex/simplex.h"
#include <suitesparse/cs.h>

#define EPSILON 0
#define MU 1
#define TRUE 1
#define FALSE 0

/*
Given a 2-chain, its boundary, and material properties, compute the
maxwell update matrix 2*I-d*eps^(-1)*d^T*mu^(-1).
*/
cs_di* maxwell_matrix(const kchn* chn,const bdry* bdry);

/*
Given a list of vertices and a k-chain, compute the barycenters of
the k-chain simplices.
*/
float* barycenters(const kchn* chn,const mesh* msh);

/*

*/
kchn* kchn_idcs(const kchn* chn);

int cs_gaxpy_float(const cs* A,const float* x,float *y);

#endif

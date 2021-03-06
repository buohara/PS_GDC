#ifndef ACSIMSEP_H
#define ACSIMSEP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <suitesparse/cs.h>
#include <suitesparse/klu.h>
#include "ps.h"
#include "opt.h"

#define MAX_T 1800.0
#define BIG_EPS 1e-6
#define SMALL_EPS 1e-12

#ifdef __cplusplus
extern "C"{
#endif

/*
build Ybus matrices.
*/
cs_ci** get_ybus(PS *ps);

/*

*/
int acpf(PS *ps);

#ifdef __cplusplus
}
#endif

#endif

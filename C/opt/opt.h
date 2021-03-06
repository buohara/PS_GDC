#ifndef OPT_H
#define OPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <suitesparse/cs.h>
#include <suitesparse/klu.h>

#define EPS 1e-5
#define C1 1e-4
#define C2 0.9
#define ITER_MAX 20
#define ALPHA_MAX 10.0

#ifdef __cplusplus
extern "C"{
#endif

/*
function pointer types for objective, gradient, and hessian functions.
*/
typedef double (*OBJ_FUNC)(const double*,void*);
typedef double* (*GRAD_FUNC)(const double*,double*,void*);
typedef int (*HESS_FUNC)(const double*,cs_di*,void*);
typedef cs_di* (*INIT_HESS_FUNC)(const double*,void*); 

/*
optimization routine that computes a search direction p and then approximately minimizes an
objective function f along this direction using the line search method.
*/
double* ls_opt(const int n,const double* x0,const OBJ_FUNC f,const GRAD_FUNC g,const HESS_FUNC h,const INIT_HESS_FUNC init_hess,void* data);

/*
line search subroutine. computes an approximate solution the one-dimensional minimization 
problem f(alpha)=f(x+alpha*p) for alpha along the direction p using cubic interpolation.
*/
double line_search(const int n,const double *x0,const double *p,const OBJ_FUNC f,const GRAD_FUNC g,void* data);

/*
simple dot product
*/
double dot(const int n,const double *x1,const double *x2);

/*
check if a vector's entries are sufficiently close to zero
*/
int is_zero(const int n,const double* x);

/*
add two vectors using x_out=x1+alpha*x2
*/
void add(const int n,const double* x1,const double* x2,double alpha,double* x_out);

/*
calculate a finite-difference derivative of a vector valued function
*/
cs_di* check_derivative(const int n,const GRAD_FUNC g,const double *x0,void* data);

#ifdef __cplusplus
}
#endif

#endif

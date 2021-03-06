#include "opt.h"

/*
simple dot product
*/
double dot(const int n,const double *x1,const double *x2)
{
	if(!(x1&&x2))return 0.0;
	int i;
	double result=0.0;
	for(i=0;i<n;i++)result+=x1[i]*x2[i];
	return result;
};

/*
check if a vector's entries are sufficiently close to zero
*/
int is_zero(const int n,const double* x)
{
	if(!x)return 0;
	int i,is_zero=1;
	for(i=0;i<n;i++)is_zero&=(fabs(x[i])<EPS);
	return is_zero;
};

/*
add two vectors using x_out=x1+alpha*x2
*/
void add(const int n,const double* x1,const double* x2,double alpha,double* x_out)
{
	if(!(x1&&x2&&x_out))return;
	int i;
	for(i=0;i<n;i++)x_out[i]=x1[i]+alpha*x2[i];
};

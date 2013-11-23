#include "acsimsep.h"

//forward delcarations
static double snorm(const double* v,void* data);
static double* s(const double* v,double* s_out,void* data);
static int jac(const double* v,cs_di* jac,void* data);
static cs_di* init_jac(const double* v,void* data);

/*
return g^Tg with g=s-(sg-sd)
*/
double snorm(const double* v,void* data)
{
	return 0.0;
}

/*
return s-(sg-sd)
*/
double* s(const double* v,double* s_out,void* data)
{
	return NULL;
}

/*
return j=ds/dv
*/
int jac(const double* v,cs_di* jac,void* data)
{
	return -1;
}

/*
initialize j=ds/dv
*/
cs_di* init_jac(const double* v,void* data)
{
	return NULL;
}

int acpf(PS *ps)
{
	return -1;
}

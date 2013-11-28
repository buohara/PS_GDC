#include "opt.h"

static double obj_func(const double* v,void* data);
static double* grad_func(const double* v,double* curr,void* data);
static int hess_func(const double* v,cs_di* hess,void* data);
static cs_di* init_hess(const double*,void* data);

double obj_func(const double* v,void* data)
{
	return 0.0;
}

double* grad_func(const double* v,double* curr,void* data)
{
	return NULL;
}

static int hess_func(const double* v,cs_di* hess,void* data)
{
	return 0;
}

static cs_di* init_hess(const double* v,void* data)
{
	return NULL;
}

int main(int argc,char** argv)
{
	cs_ci *ytmp=cs_ci_spalloc(3,3,0,1,1),*ybus;
	cs_ci_entry(ytmp,0,0,15-35*I);
	cs_ci_entry(ytmp,0,1,-10-+20*I);
	cs_ci_entry(ytmp,0,2,-5+15*I);
	cs_ci_entry(ytmp,1,0,-10+20*I);
	cs_ci_entry(ytmp,1,1,30-60*I);
	cs_ci_entry(ytmp,1,2,-20+10*I);
	cs_ci_entry(ytmp,2,0,-5+15*I);
	cs_ci_entry(ytmp,2,1,-20+40*I);
	cs_ci_entry(ytmp,2,2,25-55*I);

	cs_ci_print(ytmp,0);
	
	cs_ci_spfree(ytmp);
	return 0;	
}

#include "opt.h"

static double obj_func(const double* v,void* data);
static double* grad_func(const double* v,double* curr,void* data);
static int hess_func(const double* v,cs_di* hess,void* data);
static cs_di* init_hess(const double*,void* data);

typedef struct
{
	cs_ci* ybus;
	double* pq;
}PS_DATA;

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
	//initialize variables
	int n=3,ref_bus=0,npv=1,pv[1]={1};
	cs_ci *ybus=(cs_ci*)data;
	cs_di *jac_tmp=cs_di_spalloc(n,n,0,1,1),jac;
	int i,j,*p=ybus->p,*r=ybus->r;
	complex double *x=ybus->x;	
	
	/*
	build dI/dV component. start with ybus piece
	*/
	for(i=0;i<n;i++)
	{
		for(j=p[i];j<p[i+1];j++)
		{
			cs_entry(jac_temp,2*r[j],2*i,creal(x[j]));
			cs_entry(jac_temp,2*r[j]+1,2*i,cimag(x[j]));
			cs_entry(jac_temp,2*r[j],2*i+1,-cimag(x[j]));
			cs_entry(jac_temp,2*r[j]+1,2*i+1,creal(x[j]));
		}
	}
	
	//update diagonals of dI/dV
	for(i=0;i<3;i++)
	{
	
	}
	
	/*
	build dc/dv
	*/
	cs_entry(jac_tmp,2*n,2*ref_bus,2*v[2*ref_bus]);
	cs_entry(jac_tmp,2*n,2*ref_bus+1,2*v[2*ref_bus+1]);
	cs_entry(jac_tmp,2*n+1,2*ref_bus,2*v[2*ref_bus]);
	for(i=0;i<npv;i++)
	{
		cs_entry(jac_tmp,2*(n+1+i),2*pv[i],2*v[2*pv[i]]);
		cs_entry(jac_tmp,2*(n+1+i),2*pv[i]+1,2*v[2*pv[i]+1]);
	}
	
	/*
	build dI/dS
	*/
	cs_entry(jac_tmp,0,7,v[0]/(v[0]*v[0]+v[1]*v[1]));
	cs_entry(jac_tmp,0,8,v[1]/(v[0]*v[0]+v[1]*v[1]));
	cs_entry(jac_tmp,1,7,v[1]/(v[0]*v[0]+v[1]*v[1]));
	cs_entry(jac_tmp,1,8,-v[0]/(v[0]*v[0]+v[1]*v[1]));
	
	//clean up and return
	jac=cs_compress(jac_tmp);
	cs_spfree(jac_tmp);
	return jac;
}

int main(int argc,char** argv)
{
	PS_DATA data;

	cs_ci *ytmp=cs_ci_spalloc(3,3,0,1,1);
	cs_ci_entry(ytmp,0,0,15-35*I);
	cs_ci_entry(ytmp,0,1,-10-+20*I);
	cs_ci_entry(ytmp,0,2,-5+15*I);
	cs_ci_entry(ytmp,1,0,-10+20*I);
	cs_ci_entry(ytmp,1,1,30-60*I);
	cs_ci_entry(ytmp,1,2,-20+10*I);
	cs_ci_entry(ytmp,2,0,-5+15*I);
	cs_ci_entry(ytmp,2,1,-20+40*I);
	cs_ci_entry(ytmp,2,2,25-55*I);

	data.pq=malloc(6*sizeof(double));
	

	data.ybus=cs_ci_compress(ytmp);
	cs_ci_spfree(ytmp);
	
	cs_ci_spfree(data.ybus);
	free(data.pq);
	return 0;	
}

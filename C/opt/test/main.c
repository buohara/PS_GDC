#include "opt.h"

//data defining quadratic problem x^tAx+bx
typedef struct{
	int n;
	cs_di *A;
	double *b;
}quad_prob;

//return x^tAx+bx
double objective(const double* x,void* data)
{
	return sin(x[0])+cos(x[1]);
}

//return Ax+b
double* gradient(const double* x,double* x_out,void* data)
{
	x_out[0]=cos(x[0]);
	x_out[1]=-sin(x[1]);
}

//hessian initialization function
cs_di* init_hess(const double* x0,void* data)
{
	cs_di *A,*B=cs_spalloc(2,2,1,1,1);
	cs_entry(B,0,0,-sin(x0[0]));
	cs_entry(B,1,1,-cos(x0[1]));
	A=cs_compress(B);
	cs_spfree(B);
	return A;
} 

//hessian function
int hessian(const double* x,cs_di* H,void* data)
{
	double *xh=H->x;
	xh[0]=-sin(x[0]);
	xh[1]=-cos(x[1]);
} 

//return A
int main(int argc,char** argv)
{
	int i,n=2;
	cs_di *A,*B=cs_spalloc(2,2,1,1,1);
	cs_entry(B,0,0,1.8);
	cs_entry(B,0,1,-0.4);
	cs_entry(B,1,0,-0.4);
	cs_entry(B,1,1,-1.2);
	A=cs_compress(B);
	cs_spfree(B);
	
	double *b=malloc(2*sizeof(double)),x0[2]={-2.0,3.0};
	b[0]=1.0;b[1]=2.0;
	
	quad_prob *prob=malloc(sizeof(quad_prob));
	prob->n=2;
	prob->A=A;
	prob->b=b;
	
	double *x_star=ls_opt(n,x0,objective,gradient,hessian,init_hess,(void*)prob);
	for(i=0;i<2;i++)printf("%lg ",x_star[i]);
	
	free(x_star);
	cs_spfree(A);
	free(b);
	free(prob);
	return 0;
}

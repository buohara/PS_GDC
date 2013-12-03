#include "opt.h"

cs_di* check_derivative(const int n,const GRAD_FUNC g,const double* x0,void* data)
{
	//init variables
	double h=1e-6,*x=malloc(n*sizeof(double)),*g1=malloc(n*sizeof(double)),
	*g2=malloc(n*sizeof(double));
	int i,j;
	cs_di *jac_tmp=cs_spalloc(n,n,0,1,1),*jac;
	
	//copy evaluation point
	for(i=0;i<n;i++)x[i]=x0[i];
	
	//compute jacobian one column at a time
	for(i=0;i<n;i++)
	{
		x[i]+=h;
		g(x,g1,data);
		x[i]-=(2*h);
		g(x,g2,data);
		x[i]+=h;
		for(j=0;j<n;j++)if(fabs(g2[j]=(g1[j]-g2[j])/(2.0*h))>1e-12)cs_entry(jac_tmp,j,i,g2[j]);
	}
	
	//sort, cleanup, return
	jac=cs_compress(jac_tmp);
	cs_spfree(jac_tmp);
	jac_tmp=cs_transpose(jac,1);
	cs_spfree(jac);
	jac=cs_transpose(jac_tmp,1);
	free(g1);
	free(g2);
	free(x);
	return jac;
}

#include "opt.h"

double* ls_opt(const int n,const double* x0,const OBJ_FUNC f,const GRAD_FUNC g,const HESS_FUNC h, const INIT_HESS_FUNC init_hess,void* data)
{
	//check inputs
	if(!(x0&&f&&g&&h))return NULL;
	
	//initialize variables
	int n_iters=0,ok,i;
	double f1,f2,g0,alpha,norm_p,*x1=malloc(n*sizeof(double)),*x2=malloc(n*sizeof(double)),*grad=malloc(n*sizeof(double)),*p=malloc(n*sizeof(double)),*x_tmp;
	cs_di *hess=init_hess(x0,data);
	klu_common common;
	klu_symbolic *symb=NULL;
	klu_numeric *num=NULL;
	
	//copy over starting point to x1
	for(i=0;i<n;i++)x1[i]=x0[i];
	
	/*
	evaluate function and gradient at starting point
	*/
	f1=f(x1,data);
	g(x1,grad,data);
	
	//initialize KLU
	ok=klu_defaults(&common);
	
	/*
	otherwise, find a search direction and start line search
	*/
	do
	{
		//compute the Hessian
		h(x1,hess,data);
		
		/*
		flip the sign of g so we solve -g=Hp 
		*/
		for(i=0;i<n;i++)p[i]=-grad[i];
		
		cs_print(hess,0);
		
		//perform linear solve for p
		symb=klu_analyze(n,hess->p,hess->i,&common);
		num=klu_factor(hess->p,hess->i,hess->x,symb,&common);
		printf("%d\n",common.status);
		klu_solve(symb,num,n,1,p,&common);
		
		//free up klu resources
		klu_free_symbolic(&symb,&common);
		klu_free_numeric(&num,&common);
		
		/*
		make sure p is a descent direction. if not, bail and use
		gradient descent.
		*/
		g0=dot(n,grad,p);
		if(g0>0){for(i=0;i<n;i++)p[i]=-grad[i];}
		
		//normalize the search direction
		norm_p=0.0;
		for(i=0;i<n;i++)norm_p+=p[i]*p[i];
		norm_p=1.0/sqrt(norm_p);
		for(i=0;i<n;i++)p[i]*=norm_p;
		
		//perform line search to get step length
		alpha=line_search(n,x1,p,f,g,data);
		
		/*
		calculate gradient at new point, increment iteration counter, and continue
		*/
		add(n,x1,p,alpha,x2);
		f1=f2;
		f2=f(x2,data);
		g(x2,grad,data);
		x_tmp=x1;x1=x2;x2=x_tmp;
		n_iters++;
	}while(fabs(f2-f1)>EPS&&n_iters<ITER_MAX);
	
	printf("Number of steps: %d\n",n_iters);
	
	//exit if a zero could not be found in the max number of iterations
	if(n_iters==ITER_MAX)
	{
		printf("Failed to locate a zero in %d iterations. Exiting...",ITER_MAX);
		free(grad);
		free(p);
		free(x1);
		free(x2);
		cs_spfree(hess);
		exit(8);
	}
	
	//free memory and return
	free(grad);
	free(p);
	free(x2);
	cs_spfree(hess);
	return x1;
}

#include "opt.h"

//compute the minimum of a cubic spline interpolant
inline static double min_cubic(double a1,double a2,double f1,double f2,double g1,double g2)
{
	double d1,d2,adiff=a2-a1;
	d1=g1+g2+3.0*(f1-f2)/adiff;
	d2=((adiff>0.0)-(adiff<0.0))*sqrt(d1*d1-g1*g2);
	return a2-adiff*(g2+d2-d1)/(g2-g1+2.0*d2);
}

/*
line search subroutine. computes an approximate solution the one-dimensional minimization 
problem f(alpha)=f(x+alpha*p) for alpha along the direction p. 
*/
double line_search(const int n,const double *x0,const double *p,const OBJ_FUNC f,const GRAD_FUNC g,void* data)
{
	//check inputs
	if(!(x0&&p&&data))return -1.0;
	
	//initialize variables
	double a1=0.0,a2=1.0,at,f0,f1,f2,ft,g0,g1,g2,gt,*grad=malloc(n*sizeof(double)),*x=malloc(n*sizeof(double));
	int i,outer_iters=0,inner_iters=0;
	f0=f1=f(x0,data);
	g(x0,grad,data);
	g0=g1=dot(n,grad,p);
	
	//start line search
	while(outer_iters<ITER_MAX)
	{		
		//evaluate f and g at new point
		add(n,x0,p,a2,x);
		f2=f(x,data);
		
		g(x,grad,data);
		g2=dot(n,grad,p);
		
		/*
		check the wolfe conditions. if satisfied, we're done.
		*/
		if(f2<=f0+C1*g0&&fabs(g2)<=-C2*g0)
		{
			free(grad);
			free(x);	
			return a2;
		}
		
		/*
		if the function violated the sufficient decrease condition or its slope became positive, 
		there must be a point that satisfies the wolfe conditions in the bracket. interpolate and
		find it.
		*/
		if(f2>f0+C1*g0||g2>=0)
		{
			while(inner_iters<ITER_MAX)
			{
				printf("Voltage/Power:\n");
				for(i=0;i<n;i++)printf("%lg ",x[i]);
				printf("\nMismatch:\n");
				for(i=0;i<n;i++)printf("%lg ",grad[i]);
				printf("\n\n");
			
				//solve for minimum of cubic interpolant
				at=min_cubic(a1,a2,f1,f2,g1,g2);
				
				//make sure we aren't taking steps that are too small
				at=fmin(fmax(at,a1+(a2-a1)*1.5e-1),a2-(a2-a1)*1.5e-1);
				
				//move to trial point and evaluate f and g
				add(n,x0,p,at,x);
				ft=f(x,data);
				g(x,grad,data);
				gt=dot(n,grad,p);
				
				printf("step: %lg %lg %lg\nf: %lg %lg %lg\ng: %lg %lg %lg\n\n",a1,at,a2,f1,ft,f2,g1,gt,g2);
				
				//check wolfe conditions
				if(/*ft<=f0+C1*g0&&*/fabs(gt)<=-C2*g0)
				{
					free(grad);
					free(x);
					return at;
				}
				
				//adjust bracket
				if(gt>0){f2=ft;g2=gt;a2=at;}
				else{f1=ft;g1=gt;a1=at;}
				
				//iterate our counter
				inner_iters++;
			}
			break;
		}
		
		/*
		otherwise, we decreased enough but our slope hasn't leveled out, so slide our
		bracket forward
		*/
		a1=a2;
		a2+=(ALPHA_MAX-a2)*0.5;
		f1=f2;
		g1=g2;
		outer_iters++;
	}
	
	//we failed. print error
	printf("Line search failed to find an adequate step length in %d iterations. Exiting...\n",ITER_MAX);
	
	//free memory and exit
	free(grad);
	free(x);
	exit(8);
}


#include "opt.h"
#define complex _Complex

//forward declarations
static double obj_func(const double* v,void* data);
static double* grad_func(const double* v,double* curr,void* data);
static int hess_func(const double* v,cs_di* hess,void* data);
static cs_di* init_hess(const double*,void* data);

//data structure for problem data
typedef struct
{
	cs_ci* ybus;
	double* pq;
}PS_DATA;

//objective function (i;c)^T(i;c)
double obj_func(const double* v,void* data)
{
	//initialize variables
	int i;
	double result=0.0,*icurr=malloc(9*sizeof(double));
	grad_func(v,icurr,data);
	for(i=0;i<9;i++)result+=0.5*icurr[i]*icurr[i];
	return result;
}

//gradient function i;c
double* grad_func(const double* v,double* curr,void* data)
{
	//initialize variables
	int n=3,i,ref_bus=0,npv=1,pv[1]={1};
	PS_DATA *ps_data=(PS_DATA*)data;
	cs_ci *ybus=ps_data->ybus;
	double *pq=ps_data->pq,vmag=1.02*1.02,vr,vi,fac,P,Q;
	
	/*
	update slack and pv bus ps and qs, then copy over values for
	a gaxpy.
	*/
	pq[0]=v[6];
	pq[1]=v[7];
	pq[3]=v[8];
	for(i=0;i<3;i++)
	{
		vr=v[2*i];
		vi=v[2*i+1];
		P=pq[2*i];
		Q=pq[2*i+1];
		fac=1.0/(vr*vr+vi*vi);
		curr[2*i]=0.0-(P*vr+Q*vi)*fac;
		curr[2*i+1]=0.0-(P*vi-Q*vr)*fac;
	}
	
	//compute i=Yv-s
	cs_ci_gaxpy(ybus,(void*)v,(void*)curr);
	
	//compute constraints
	curr[6]=v[0]*v[0]+v[1]*v[1]-vmag;
	curr[7]=v[0]*v[0]-vmag;
	curr[8]=v[2]*v[2]+v[3]*v[3]-vmag;
	
	//return
	return NULL;
}

//hessian (jacobian) update
int hess_func(const double* v,cs_di* hess,void* data)
{
	//initialize variables
	PS_DATA *ps_data=(PS_DATA*)data;
	cs_ci *ybus=ps_data->ybus;
	int i,j,k,*p=ybus->p,*r=ybus->i,*rup,*pup,n=3,ref_bus=0,
	npv=1,nhess=2*(n+1)+npv,pv[1]={1};
	double vr,vi,fac,g,b,P,Q,*pq=ps_data->pq,*x,*xup;
	cs_di *jac_tmp=cs_di_spalloc(nhess,nhess,4*(n+npv+1)+3,1,1),*jac;
	complex double *y=ybus->x;
	
	/*
	copy over slack and pv bus ps and qs.
	*/
	pq[0]=v[6];
	pq[1]=v[7];
	pq[3]=v[8];
	
	/*
	update di/dv diagonals
	*/
	for(i=0;i<n;i++)
	{
		for(j=p[i];j<p[i+1];j++)
		{			
			if(r[j]==i)
			{
				vr=v[2*i];
				vi=v[2*i+1];
				g=creal(y[j]);
				b=cimag(y[j]);
				P=pq[2*i];
				Q=pq[2*i+1];
				fac=(vi*vi-vr*vr)/(vr*vr+vi*vi);			
				cs_entry(jac_tmp,2*i,2*i,g-P*fac);
				cs_entry(jac_tmp,2*i+1,2*i,b+Q*fac);
				cs_entry(jac_tmp,2*i,2*i+1,-b+Q*fac);
				cs_entry(jac_tmp,2*i+1,2*i+1,g+P*fac);
			}
		}
	}
	
	/*
	loop through slack and pv buses and update dc/dv and di/ds. start
	with swing bus.
	*/
	vr=v[2*ref_bus];
	vi=v[2*ref_bus+1];
	fac=-1.0/(vr*vr+vi*vi);
	
	//dc/dv
	cs_entry(jac_tmp,6,0,2*vr);
	cs_entry(jac_tmp,6,1,2*vi);
	cs_entry(jac_tmp,7,0,2*vr);
	
	//di/ds
	cs_entry(jac_tmp,0,6,vr*fac);
	cs_entry(jac_tmp,0,7,vi*fac);
	cs_entry(jac_tmp,1,6,vi*fac);
	cs_entry(jac_tmp,1,7,-vr*fac);
	
	/*
	do pv buses.
	*/
	for(i=0;i<npv;i++)
	{
		vr=v[2*pv[i]];
		vi=v[2*pv[i]+1];
		fac=-1.0/(vr*vr+vi*vi);
	
		//dc/dv
		cs_entry(jac_tmp,2*(n+1)+i,2*pv[i],2*vr);
		cs_entry(jac_tmp,2*(n+1)+i,2*pv[i]+1,2*vi);
		
		//di/ds
		cs_entry(jac_tmp,2*pv[i],2*(n+1)+i,vi*fac);
		cs_entry(jac_tmp,2*pv[i]+1,2*(n+1)+i,-vr*fac);
	}
	
	//compress and sort updates
	jac=cs_compress(jac_tmp);
	cs_spfree(jac_tmp);
	jac_tmp=cs_transpose(jac,1);
	cs_spfree(jac);
	jac=cs_transpose(jac_tmp,1);
	cs_spfree(jac_tmp);
	
	//copy updates into hess
	p=hess->p;
	r=hess->i;
	x=hess->x;
	pup=jac->p;
	rup=jac->i;
	xup=jac->x;
	for(i=0;i<nhess;i++)
	{
		k=p[i];
		for(j=pup[i];j<pup[i+1];j++)
		{
			while(r[k]<rup[j])k++;
			x[k++]=xup[j];
		}	
	}
	
	//clean up and return
	cs_spfree(jac);
	return 0;
}

//build initial hessian (jacobian)
cs_di* init_hess(const double* v,void* data)
{
	//initialize variables
	PS_DATA *ps_data=(PS_DATA*)data;
	cs_ci *ybus=ps_data->ybus;
	int i,j,*p=ybus->p,*r=ybus->i,n=3,ref_bus=0,npv=1,pv[1]={1};
	cs_di *jac_tmp=cs_di_spalloc(9,9,0,1,1),*jac;
	double vr,vi,fac,*pq=ps_data->pq,P,Q,g,b;
	complex double *x=ybus->x;
	
	/*
	build dI/dV component. start with ybus piece
	*/
	for(i=0;i<n;i++)
	{
		for(j=p[i];j<p[i+1];j++)
		{
			g=creal(x[j]);
			b=cimag(x[j]);
			cs_entry(jac_tmp,2*r[j],2*i,g);
			cs_entry(jac_tmp,2*r[j]+1,2*i,b);
			cs_entry(jac_tmp,2*r[j],2*i+1,-b);
			cs_entry(jac_tmp,2*r[j]+1,2*i+1,g);
			
			//initialize diagonals
			if(r[j]==i)
			{
				vr=v[2*i];
				vi=v[2*i+1];
				P=pq[2*i];
				Q=pq[2*i+1];
				fac=(vi*vi-vr*vr)/(vr*vr+vi*vi);		
				cs_entry(jac_tmp,2*i,2*i,-P*fac);
				cs_entry(jac_tmp,2*i+1,2*i,Q*fac);
				cs_entry(jac_tmp,2*i,2*i+1,Q*fac);
				cs_entry(jac_tmp,2*i+1,2*i+1,P*fac);
			}
		}
	}
	
	/*
	loop through slack and pv buses to build dc/dv and di/ds. start
	with swing bus.
	*/
	vr=v[0];
	vi=v[1];
	fac=-1.0/(vr*vr+vi*vi);
	
	//dc/dv
	cs_entry(jac_tmp,6,0,2*vr);
	cs_entry(jac_tmp,6,1,2*vi);
	cs_entry(jac_tmp,7,0,2*vr);
	
	//di/ds
	cs_entry(jac_tmp,0,6,vr*fac);
	cs_entry(jac_tmp,0,7,vi*fac);
	cs_entry(jac_tmp,1,6,vi*fac);
	cs_entry(jac_tmp,1,7,-vr*fac);
	
	/*
	do pv buses.
	*/
	for(i=0;i<npv;i++)
	{
		vr=v[2*pv[i]];
		vi=v[2*pv[i]+1];
		fac=-1.0/(vr*vr+vi*vi);
	
		//dc/dv
		cs_entry(jac_tmp,2*(n+1)+i,2*pv[i],2*vr);
		cs_entry(jac_tmp,2*(n+1)+i,2*pv[i]+1,2*vi);
		
		//di/ds
		cs_entry(jac_tmp,2*pv[i],2*(n+1)+i,vi*fac);
		cs_entry(jac_tmp,2*pv[i]+1,2*(n+1)+i,-vr*fac);
	}
	
	//compress and sort
	jac=cs_compress(jac_tmp);
	cs_spfree(jac_tmp);
	jac_tmp=cs_transpose(jac,1);
	cs_spfree(jac);
	jac=cs_transpose(jac_tmp,1);
	cs_spfree(jac_tmp);
	cs_dupl(jac);
	
	//return
	return jac;
}

//main
int main(int argc,char** argv)
{
	PS_DATA data;

	cs_ci *ytmp=cs_ci_spalloc(3,3,0,1,1);
	cs_ci_entry(ytmp,0,0,15-35*I);
	cs_ci_entry(ytmp,0,1,-10+20*I);
	cs_ci_entry(ytmp,0,2,-5+15*I);
	cs_ci_entry(ytmp,1,0,-10+20*I);
	cs_ci_entry(ytmp,1,1,30-60*I);
	cs_ci_entry(ytmp,1,2,-20+40*I);
	cs_ci_entry(ytmp,2,0,-5+15*I);
	cs_ci_entry(ytmp,2,1,-20+40*I);
	cs_ci_entry(ytmp,2,2,25-55*I);

	data.pq=malloc(6*sizeof(double));
	data.pq[0]=0.5;
	data.pq[1]=0.07;
	data.pq[2]=0.5;
	data.pq[3]=0.55;
	data.pq[4]=-1.0;
	data.pq[5]=-0.6;
	
	double *x0=malloc(9*sizeof(double));
	x0[0]=1.02;
	x0[1]=0.1;
	x0[2]=1.02;
	x0[3]=-0.1;
	x0[4]=1.0;
	x0[5]=-0.17;
	x0[6]=0.51;
	x0[7]=0.07;
	x0[8]=0.55;
	
	data.ybus=cs_ci_compress(ytmp);
	cs_ci_spfree(ytmp);
	
	cs_di *jac=check_derivative(9,grad_func,x0,(void*)&data);
	cs_print(jac,0);
	double *result=ls_opt(9,x0,obj_func,grad_func,hess_func,init_hess,(void*)&data);
	
	cs_ci_spfree(data.ybus);
	cs_spfree(jac);
	free(data.pq);
	free(x0);
	free(result);
	return 0;	
}

#include "dcsimsep.h"

/*
check relays for overcurrent and trip branches as needed
*/
double update_relays(PS *ps,Outage *outages,double t)
{
	//check inputs
	if(!(ps&&outages))return -1.0;
		
	//prep variables
	int i,nbranch=ps->nbranch,narea=ps->narea;
	BUS *buses=ps->buses;
	BRANCH *branches=ps->branches;
	double bmva_inv=1.0/ps->base_mva,imag,imax,t_relay,threshold,threshold_dist,
	excess,t_min=INFINITY,*ovld_ts=malloc(nbranch*sizeof(double));
	
	/*
	loop through branches and compute overload times. record the smallest
	as we go along
	*/
	for(i=0;i<nbranch;i++)
	{
		if(!branches[i].status)continue;
		imag=branches[i].imag;
		imax=branches[i].rate_b*bmva_inv;
		threshold=branches[i].threshold;
		threshold_dist=threshold-branches[i].state_a;
		ovld_ts[i]=INFINITY;
		if(imag<imax)
		{
			branches[i].state_a-=BIG_EPS;
			continue;
		}
		ovld_ts[i]=threshold_dist/(imag-imax);
		if(ovld_ts[i]<t_min)t_min=ovld_ts[i];
	}
	
	if(t_min==INFINITY)return t_min;
	
	/*
	loop through the branches again. if the overload time equals the minimum trip
	time for its area, trip it
	*/
	for(i=0;i<nbranch;i++)
	{
		if(!branches[i].status)continue;	
		if(ovld_ts[i]==t_min)
		{
			branches[i].status=0;
			add_branch_out(outages,branches[i].from,branches[i].to,t+t_min);
		}
	}
	
	/*
	update relay states based on excess current
	*/
	for(i=0;i<nbranch;i++)
	{
		if(!branches[i].status)continue;
		imag=branches[i].imag;
		imax=branches[i].rate_b*bmva_inv;
		branches[i].state_a+=(imag-imax)*t_min+SMALL_EPS;
		if(branches[i].state_a<0.0)branches[i].state_a=0.0;
	}
	
	//free up memory and return
	free(ovld_ts);
	return t_min;
}

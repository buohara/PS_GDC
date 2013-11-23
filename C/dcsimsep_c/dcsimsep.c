#include "dcsimsep.h"

/*
run dc cascading failure simulation
*/
int dcsimsep(PS *ps,Outage *outages)
{
	//check inputs
	if(!(ps&&outages))return -1;

	//prep variables
	int narea,i;
	double t=1.0,dt;
	
	/*
	perform exogenous branch trips. find subgraphs, and redispatch generation as needed
	*/
	narea=1;
	trip_branches(ps,outages);
	find_subgraphs(ps);
	if(narea<ps->narea)
	{
		narea=ps->narea;
		redispatch(ps);
	}
	
	/*
	main while loop that takes us from 0 to 30 minutes
	*/
	while(t<MAX_T)
	{
		//run load flow
		dcpf(ps);
		
		//check if any line overloads tripped relays
		dt=update_relays(ps,outages,t);
		t+=dt;
		
		//check for any new grid separation and redispatch if so
		find_subgraphs(ps);
		if(narea<ps->narea)
		{
			narea=ps->narea;
			redispatch(ps);	
		}
	}
	
	//finish up and return
	return 0;
}

#include "dcsimsep.h"

int redispatch(PS *ps)
{
	/*
	check inputs and prep variables
	*/
	if(!ps)return -1;
	int i,j,narea=ps->narea,ngen=ps->ngen,nshunt=ps->nshunt,*gen_areas=malloc(ngen*sizeof(int)),
	*load_areas=malloc(nshunt*sizeof(int)),gen_cnt=0;
	double dt=6.,*max_ramp_rates=malloc(ngen*sizeof(double)),*gen_sums=malloc(narea*sizeof(double)),
	*load_sums=malloc(narea*sizeof(double)),pgd_frac=0.,ramp,bmva_inv=1.0/ps->base_mva,ramp_sum=0.,
	pmin=0.,pmax=0.,*pg0=malloc(ngen*sizeof(double));
	BUS *buses=ps->buses;
	GEN *gens=ps->gens,*min_gen=NULL;
	SHUNT *shunts=ps->shunts;
	
	/*
	clear out generation and load sums for each area
	*/
	for(i=0;i<narea;i++)gen_sums[i]=load_sums[i]=0.;
	
	/*
	set max amount of gen ramping to 5% of max generation per minute
	times 10.0 seconds. grab each generator's area and sum the generation
	while we're at it.
	*/
	for(i=0;i<ngen;i++)
	{
		max_ramp_rates[i]=gens[i].pmax*0.05;
		gen_areas[i]=buses[gens[i].bus].area;
		gen_sums[gen_areas[i]]+=gens[i].status*gens[i].pg;
		gen_cnt+=gens[i].status;
		pg0[i]=gens[i].pg;
	}
	
	/*
	grab each load's area and sum the loads.
	*/
	for(i=0;i<nshunt;i++)
	{
		load_areas[i]=buses[shunts[i].bus].area;
		load_sums[load_areas[i]]+=shunts[i].p*shunts[i].frac_d;
	}
	
	/*
	balance each area's generation and load
	*/
	for(i=0;i<narea;i++)
	{
		/*
		check for negative loads. disconnect all loads and generators
		and move on if found.
		*/
		if(load_sums[i]<0)
		{
			for(j=0;j<ngen;j++)
			{
				if(gen_areas[j]!=i)continue;
				gens[j].status=0;
			}
			for(j=0;j<nshunt;j++)
			{
				if(load_areas[j]!=i)continue;
				shunts[j].frac_d=0;
			}
			continue;
		}
		
		/*
		if generation exceeds load, decrease generation
		*/	
		while((gen_sums[i]-load_sums[i])>BIG_EPS)
		{		
			/*
			first, try ramping down generation
			*/
			ramp_sum=0.;
			for(j=0;j<ngen;j++)
			{
				if(!(gen_areas[j]==i&&gens[j].status))continue;
				pmin=fmax(gens[j].pmin,pg0[j]-max_ramp_rates[j]);
				if(gens[j].pg>pmin)ramp_sum+=max_ramp_rates[j];
			}
			
			//break if we couldn't find any generators to ramp
			if(ramp_sum==0.)break;
			
			//otherwise, scale down generation
			pgd_frac=fabs((load_sums[i]-gen_sums[i])/ramp_sum);
			for(j=0;j<ngen;j++)
			{
				if(!(gen_areas[j]==i&&gens[j].status))continue;
				pmin=fmax(gens[j].pmin,pg0[j]-max_ramp_rates[j]);
				if(gens[j].pg>pmin)
				{
					ramp=fmin(max_ramp_rates[j]*pgd_frac,gens[j].pg-pmin);
					gens[j].pg-=ramp;
					gen_sums[i]-=ramp;
				}			
			}
		}
			
		/*
		if ramping doesn't work, start tripping generators, working up
		from the smallest
		*/
		while(((gen_sums[i]-load_sums[i])>BIG_EPS)&&gen_cnt)
		{
			for(j=0;j<ngen;j++)
			{
				if(!(gen_areas[j]==i&&gens[j].status))continue;
				if(!min_gen)
				{
					min_gen=gens+j;
					continue;
				}
				if(gens[j].pg<min_gen->pg)min_gen=gens+j;
			}
			min_gen->status=0;
			gen_sums[i]-=min_gen->pg;
			gen_cnt--;
			min_gen=NULL;
		}	
		
		/*
		if load exceeds generation, increase generation
		*/
		while((load_sums[i]-gen_sums[i])>BIG_EPS)
		{
			/*
			try ramping up generation
			*/
			ramp_sum=0.;
			for(j=0;j<ngen;j++)
			{
				if(!(gen_areas[j]==i&&gens[j].status))continue;
				pmax=fmin(gens[j].pmax,pg0[j]+max_ramp_rates[j]);
				if(gens[j].pg<pmax)ramp_sum+=max_ramp_rates[j];
			}
			
			//break if we couldn't find any generators to ramp
			if(ramp_sum==0.)break;
			
			pgd_frac=fabs((load_sums[i]-gen_sums[i])/ramp_sum);
			for(j=0;j<ngen;j++)
			{
				if(!(gen_areas[j]==i&&gens[j].status))continue;
				pmax=fmin(gens[j].pmax,pg0[j]+max_ramp_rates[j]);
				if(gens[j].pg<pmax)
				{
					ramp=fmin(max_ramp_rates[j]*pgd_frac,pmax-gens[j].pg);
					gens[j].pg+=ramp;
					gen_sums[i]+=ramp;
				}			
			}
		}
			
		//if ramping doesn't work, shed load
		if(gen_sums[i]<load_sums[i])
		{
			pgd_frac=gen_sums[i]/load_sums[i];
			load_sums[i]=0.;
			for(j=0;j<nshunt;j++)
			{
				if(load_areas[j]!=i)continue;
				shunts[j].frac_d*=pgd_frac;
				load_sums[i]+=shunts[j].frac_d*shunts[j].p;
			}
		}
	}
	
	//free memory and return
	free(gen_areas);
	free(load_areas);
	free(gen_sums);
	free(load_sums);
	free(max_ramp_rates);
	free(pg0);
}

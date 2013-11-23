#include "dcsimsep.h"

/*
solve DC load flow
*/
int dcpf(PS *ps)
{
	//check inputs and prep variables
	if(!ps)return -1;
	int i,nbus=ps->nbus,nbranch=ps->nbranch,ngen=ps->ngen,nshunt=ps->nshunt,narea=-1,*ref_buses=NULL,*bus_sub_idcs=NULL,
	*area_sizes=NULL,*area_p=NULL,area,bus,ok,from,to;
	BUS* buses=ps->buses;
	BRANCH* branches=ps->branches;
	SHUNT* shunts=ps->shunts;
	GEN* gens=ps->gens;
	cs_di **bbuses=NULL,*bbus=NULL;
	double *theta=malloc(nbus*sizeof(double)),*pgd=malloc(nbus*sizeof(double)),bmva_inv=1.0/(ps->base_mva),b;
	klu_common common;
	klu_symbolic *symb=NULL;
	klu_numeric *num=NULL;
	
	//find system islands
	narea=ps->narea;
	
	//get system Bbus matrices for each island
	bbuses=get_bbus(ps);
	ref_buses=ps->ref_buses;
	bus_sub_idcs=ps->bus_sub_idcs;
	area_sizes=ps->area_sizes;
	area_p=malloc(narea*sizeof(int));
	area_p[0]=0;
	for(i=1;i<narea;i++)area_p[i]=area_p[i-1]+area_sizes[i-1];
	
	//build Pg in B*Theta=Pg-Pd
	for(i=0;i<nbus;i++)pgd[i]=0.0;
	for(i=0;i<ngen;i++)
	{
		bus=gens[i].bus;
		area=buses[bus].area;
		if(ref_buses[area]==bus)continue;
		pgd[area_p[area]+bus_sub_idcs[bus]]+=gens[i].status*gens[i].pg;
	}
	
	//build Pd in B*Theta=Pg-Pd
	for(i=0;i<nshunt;i++)
	{
		bus=shunts[i].bus;
		area=buses[bus].area;
		if(ref_buses[area]==bus)continue;
		pgd[area_p[area]+bus_sub_idcs[bus]]-=shunts[i].p*shunts[i].frac_d;
	}
	
	//perform linear solves on each island
	ok=klu_defaults(&common);
	for(i=0;i<narea;i++)
	{
		bbus=bbuses[i];
		if(!(bbus->n))continue;
		symb=klu_analyze(bbus->n,bbus->p,bbus->i,&common);
		num=klu_factor(bbus->p,bbus->i,bbus->x,symb,&common);
		ok=klu_solve(symb,num,bbus->n,1,pgd+area_p[i],&common);
		if(!ok)printf("\nError: KLU solve failed in area %d\n",i);
		klu_free_symbolic(&symb,&common);
		klu_free_numeric(&num,&common);
	}
	
	//write bus angles to ps
	for(i=0;i<nbus;i++)
	{
		area=buses[i].area;
		buses[i].vmag=1.0;
		if(i==ref_buses[area]){buses[i].vang=0.0;continue;}
		buses[i].vang=pgd[area_p[area]+bus_sub_idcs[i]];
	}
	
	//write branch currents to ps
	for(i=0;i<nbranch;i++)
	{
		if(!branches[i].status)
		{
			branches[i].imag=0.0;
			continue;
		}
		from=branches[i].from;
		to=branches[i].to;
		b=branches[i].b;
		branches[i].imag=fabs(b*(buses[from].vang-buses[to].vang));
	}
	
	//free memory and return
	free(theta);
	free(pgd);
	free(area_p);
	//klu_free_symbolic(&symb,&common);
	//klu_free_numeric(&num,&common);
	for(i=0;i<narea;i++)cs_spfree(bbuses[i]);
	free(bbuses);
	return 0;
}

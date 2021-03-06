#include "dcsimsep.h"

/*
build Bbus matrix for each area in ps
*/
cs_di** get_bbus(PS *ps)
{
	//prep variables
	if(!ps)return NULL;
	int i,j,narea=ps->narea,nbus=ps->nbus,nbranch=ps->nbranch,ngen=ps->ngen,*ref_buses=malloc(narea*sizeof(int)),
	*area_counts=malloc(narea*sizeof(int)),*p=malloc((nbus+1)*sizeof(int)),*bus_sub_idcs=malloc(nbus*sizeof(int)),
	area,from,to;
	double *max_pgs=malloc(narea*sizeof(double)),pg,x_inv,b,bc;	
	BUS *buses=ps->buses;
	BRANCH* branches=ps->branches;
	GEN *gens=ps->gens;
	cs_di** bbuses=malloc(narea*sizeof(cs_di*)),*b_tmp;
	
	//initialize area-wide variables
	for(i=0;i<narea;i++){
		area_counts[i]=0;
		ref_buses[i]=-1;
		max_pgs[i]=0.0;
		bbuses[i]=cs_spalloc(0,0,0,1,1);
	}
	
	//find offsets into branch array for each bus's branches
	for(i=j=p[0]=0;i<nbranch;i++)while(j<branches[i].from)p[++j]=i;
	while(j<nbus+1)p[++j]=nbranch;
	
	
	//find reference buses for each area. use largest generator in each area as reference
	for(i=0;i<ngen;i++){
		pg=gens[i].pg;
		area=buses[gens[i].bus].area;
		if(pg>max_pgs[area]){max_pgs[area]=pg;ref_buses[area]=gens[i].bus;}
	}
	
	//figure out each bus's index within its subcomponent
	for(i=0;i<nbus;i++){
		area=buses[i].area;
		if(i==ref_buses[area]){bus_sub_idcs[i]=-1;continue;}
		if(ref_buses[area]==-1){ref_buses[area]=i;bus_sub_idcs[i]=-1;continue;}
		bus_sub_idcs[i]=area_counts[area]++;
	}
	
	//now loop through each bus and build bbus
	for(i=0;i<nbus;i++){
		area=buses[i].area;
		for(j=p[i];j<p[i+1];j++){
			if(!branches[j].status)continue;
			from=bus_sub_idcs[i];
			to=bus_sub_idcs[branches[j].to];
			b=branches[j].b;
			bc=-branches[j].bc;
			if(from==-1){cs_entry(bbuses[area],to,to,-b);continue;}//+0.5*b);continue;}
			if(to==-1){cs_entry(bbuses[area],from,from,-b);continue;}//+0.5*b);continue;}
			cs_entry(bbuses[area],from,to,b);
			cs_entry(bbuses[area],to,from,b);
			cs_entry(bbuses[area],from,from,-b);//+0.5*b);
			cs_entry(bbuses[area],to,to,-b);//+0.5*b);
		}
	}
	
	//compress the matrices
	for(i=0;i<narea;i++){
		b_tmp=cs_compress(bbuses[i]);
		cs_spfree(bbuses[i]);
		cs_dupl(b_tmp);
		bbuses[i]=b_tmp;
	}
	
	//clean up and return
	if(ps->area_sizes)free(ps->area_sizes);ps->area_sizes=area_counts;
	if(ps->bus_sub_idcs)free(ps->bus_sub_idcs);ps->bus_sub_idcs=bus_sub_idcs;
	if(ps->ref_buses)free(ps->ref_buses);ps->ref_buses=ref_buses;
	free(max_pgs);
	free(p);
	return bbuses;
}

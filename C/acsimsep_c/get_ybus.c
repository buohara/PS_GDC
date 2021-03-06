#include "acsimsep.h"

/*
build Ybus matrix for each area in ps
*/
cs_ci** get_ybus(PS *ps)
{
	//prep variables
	if(!ps)return NULL;
	int i,j,narea=ps->narea,nbus=ps->nbus,nbranch=ps->nbranch,ngen=ps->ngen,*ref_buses=malloc(narea*sizeof(int)),
	*area_counts=malloc(narea*sizeof(int)),*p=malloc((nbus+1)*sizeof(int)),*bus_sub_idcs=malloc(nbus*sizeof(int)),
	area,from,to;
	double *max_pgs=malloc(narea*sizeof(double)),pg,bc,r,x,ymag_inv;
	double complex y; 
	BUS *buses=ps->buses;
	BRANCH* branches=ps->branches;
	GEN *gens=ps->gens;
	cs_ci** ybuses=malloc(narea*sizeof(cs_ci*)),*y_tmp;
	
	//initialize area-wide variables
	for(i=0;i<narea;i++){
		area_counts[i]=0;
		ref_buses[i]=-1;
		max_pgs[i]=0.0;
		ybuses[i]=cs_ci_spalloc(0,0,0,1,1);
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
		if(ref_buses[area]==-1){ref_buses[area]=i;}
		bus_sub_idcs[i]=area_counts[area]++;
	}
	
	//now loop through each bus and build bbus
	for(i=0;i<nbus;i++){
		area=buses[i].area;
		for(j=p[i];j<p[i+1];j++){
			if(!branches[j].status)continue;
			from=bus_sub_idcs[i];
			to=bus_sub_idcs[branches[j].to];
			r=branches[j].r;
			x=branches[j].x;
			ymag_inv=1.0/(r*r+x*x);
			bc=-branches[j].bc;
			y=(r-I*x)*ymag_inv;
			if(from==-1){cs_ci_entry(ybuses[area],to,to,y+0.5*I*bc);continue;}
			if(to==-1){cs_ci_entry(ybuses[area],from,from,y+0.5*I*bc);continue;}
			cs_ci_entry(ybuses[area],from,to,-y);
			cs_ci_entry(ybuses[area],to,from,-y);
			cs_ci_entry(ybuses[area],from,from,y+0.5*I*bc);
			cs_ci_entry(ybuses[area],to,to,y+0.5*I*bc);
		}
	}
	
	//compress the matrices
	for(i=0;i<narea;i++){
		y_tmp=cs_ci_compress(ybuses[i]);
		cs_ci_spfree(ybuses[i]);
		cs_ci_dupl(y_tmp);
		ybuses[i]=y_tmp;
	}
	
	//clean up and return
	if(ps->area_sizes)free(ps->area_sizes);ps->area_sizes=area_counts;
	if(ps->bus_sub_idcs)free(ps->bus_sub_idcs);ps->bus_sub_idcs=bus_sub_idcs;
	if(ps->ref_buses)free(ps->ref_buses);ps->ref_buses=ref_buses;
	free(max_pgs);
	free(p);
	return ybuses;
}

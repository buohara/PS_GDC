#include "ps.h"

/*
load power system from file. i made no effort to make this robust. input
files must be in a fixed format or this won't work. pass in a file name,
returns a loaded PS or NULL on failure.
*/
PS* load_ps(const char* file_str)
{	
	//check inputs and initialize variables
	if(!file_str)return NULL;
	int i,n_elements;
	double base_mva,base_mva_inv;
	FILE *file;
	char curr_line[256];
	PS *ps;
	BUS *buses;
	BRANCH *branches;
	GEN *gens;
	SHUNT *shunts;
	//Area *areas;
	
	//try to open ps file
	file=fopen(file_str,"r");
	if(!file) return NULL;

	//initialize the power system data structure
	ps=malloc(sizeof(PS));
	
	//base mva
	if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
	sscanf(curr_line,"BASE_MVA %lg",&base_mva);
	ps->base_mva=base_mva;
	base_mva_inv=1.0/base_mva;

	//buses
	if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
	sscanf(curr_line,"BUS %d",&n_elements);
	ps->nbus=n_elements;
	buses=ps->buses=malloc(n_elements*sizeof(BUS));
	for(i=0;i<n_elements;i++){
		if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
		SCAN_BUS(curr_line,buses+i);
	}
	
	/*
	sort buses so we can search them later when building B and
	Y Bus
	*/
	qsort(ps->buses,ps->nbus,sizeof(BUS),bus_cmp);

	//branches
	if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
	sscanf(curr_line,"BRANCH %d",&n_elements);
	ps->nbranch=n_elements;
	branches=ps->branches=malloc(n_elements*sizeof(BRANCH));
	for(i=0;i<n_elements;i++){
		if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
		SCAN_BRANCH(curr_line,branches+i);
		branches[i].to=get_bus_idx(ps,branches[i].to);
		branches[i].from=get_bus_idx(ps,branches[i].from);
		branches[i].b=-1.0/branches[i].x;
		branches[i].threshold=1.5*5.0*branches[i].rate_b*base_mva_inv;
		branches[i].state_a=0.0;
		branches[i].status=1;
		if(branches[i].to<branches[i].from)SWAP_INT(branches[i].to,branches[i].from)
	}
	
	/*
	sort branches so we can do graph traversal to find system
	subcomponents
	*/
	qsort(ps->branches,ps->nbranch,sizeof(BRANCH),branch_cmp);
	
	//gens
	if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
	sscanf(curr_line,"GEN %d",&n_elements);
	ps->ngen=n_elements;
	gens=ps->gens=malloc(n_elements*sizeof(GEN));
	for(i=0;i<n_elements;i++){
		if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
		SCAN_GEN(curr_line,gens+i);
		gens[i].bus=get_bus_idx(ps,gens[i].bus);
		gens[i].pmax*=base_mva_inv;
		gens[i].pmin*=base_mva_inv;
		gens[i].pg*=base_mva_inv;
		gens[i].qg*=base_mva_inv;
	}
	
	//shunts
	if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
	sscanf(curr_line,"SHUNT %d",&n_elements);
	ps->nshunt=n_elements;
	shunts=ps->shunts=malloc(n_elements*sizeof(SHUNT));
	for(i=0;i<n_elements;i++){
		if(!fgets(curr_line,256,file)){ps_free(ps);return NULL;}
		SCAN_SHUNT(curr_line,shunts+i);
		shunts[i].bus=get_bus_idx(ps,shunts[i].bus);
		shunts[i].frac_d=1.0;
		shunts[i].p*=base_mva_inv;
		shunts[i].q*=base_mva_inv;
	}

	//close the ps file and return
	fclose(file);
	ps->narea=-1;
	ps->bus_sub_idcs=NULL;
	ps->area_sizes=NULL;
	ps->ref_buses=NULL;
	find_subgraphs(ps);
	return ps;
}

/*
free ps memory
*/
void ps_free(PS *ps)
{
	//check inputs
	if(!ps)return;
	if(ps->buses)free(ps->buses);
	if(ps->branches)free(ps->branches);
	if(ps->gens)free(ps->gens);
	if(ps->shunts)free(ps->shunts);
	//if(ps->areas)free(ps->areas);
	if(ps->bus_sub_idcs)free(ps->bus_sub_idcs);
	if(ps->area_sizes)free(ps->area_sizes);
	if(ps->ref_buses)free(ps->ref_buses);
	free(ps);
}

/*
print ps to console. passing in 1 for brief prints 10 each
of bus, branch, etc.
*/
void ps_print(const PS *ps,int brief)
{	
	//check inputs and declare variables
	if(!ps)return;
	int i,nbus=ps->nbus,nbranch=ps->nbranch,ngen=ps->ngen,nshunt=ps->nshunt;
	BUS *buses=ps->buses;
	BRANCH *branches=ps->branches;
	GEN *gens=ps->gens;
	SHUNT *shunts=ps->shunts;
	
	//header stats
	printf("\nPS: %d buses, %d branches, %d gens, %d shunts\n\n",nbus,nbranch,ngen,nshunt);
	
	//buses
	printf("Buses:\n");
	for(i=0;i<nbus;i++){
		PRINT_BUS(buses+i);
		if(brief&&(i>9)){printf("...\n");break;}
	}
	printf("\n");
	
	//branches
	printf("Branches:\n");
	for(i=0;i<nbranch;i++){
		PRINT_BRANCH(branches+i);
		if(brief&&i>9){printf("...\n");break;}
	}
	printf("\n");
	
	//gens
	printf("Gens:\n");
	for(i=0;i<ngen;i++){
		PRINT_GEN(gens+i);
		if(brief&&i>9){printf("...\n");break;}
	}
	printf("\n");
	
	//shunts
	printf("Shunts:\n");
	for(i=0;i<nshunt;i++){
		PRINT_SHUNT(shunts+i);
		if(brief&&i>9){printf("...\n");break;}
	}
	printf("\n");
	
	//return;
	return;
}

/*
get a bus's index (i.e., its row/column in Ybus) from its
ID. pass in a ps and a bus id. returns the buses index, or -1
if no match is found.
*/
int get_bus_idx(const PS *ps,int id)
{
	//check inputs
	if(!ps)return -1;
	if(!ps->buses)return -1;
	int nbus=ps->nbus;
	BUS *buses=ps->buses,*match=NULL,*key=malloc(sizeof(BUS));
	key->id=id;
	
	//search for bus id
	match=(BUS*)bsearch(key,buses,nbus,sizeof(BUS),bus_cmp);
	
	//if no match, return -1, else return pointer offset from base
	free(key);
	if(!match)return -1;
	return match-buses;
}

/*
traverse the ps and find all its connected subcomponents. modifies
areas in ps and returns number of components found.
*/
int find_subgraphs(PS *ps)
{
	//check inputs and prep variables
	if(!ps)return -1;
	if(!ps->branches)return -1;
	int i,j,*r,*p,nbus=ps->nbus,nbranch=ps->nbranch,curr_cmpt=0,*prnt,*child,*wkspc=malloc((nbus)*sizeof(int));
	prnt=child=wkspc;
	BUS *buses=ps->buses;
	BRANCH *branches=ps->branches;
	cs_di *graph_tmp=cs_spalloc(0,0,0,0,1),*graph=NULL;
	
	//mark all nodes as unvisited
	for(i=0;i<nbus;i++)buses[i].area=-1;
	
	//create the power system graph
	for(i=0;i<nbranch;i++){
		if(branches[i].status){
			cs_entry(graph_tmp,branches[i].from,branches[i].to,0.0);
			cs_entry(graph_tmp,branches[i].to,branches[i].from,0.0);
		}
	}
	graph=cs_compress(graph_tmp);
	cs_spfree(graph_tmp);
	graph_tmp=NULL;	
	p=graph->p;
	r=graph->i;
	
	//find bus offsets in graph
	/*
	for(i=j=p[0]=0;i<nbranch;i++)while(j<branches[i].from)p[++j]=i;
	p[nbus]=p[nbus-1]=nbranch;
	*/
	
	//traverse the graph
	for(i=0;i<nbus;i++){
		if(buses[i].area!=-1)continue;
		buses[i].area=curr_cmpt;
		*prnt=i;
		do{
			for(j=p[*prnt];j<p[*prnt+1];j++){
				if(buses[r[j]].area==-1){
					buses[r[j]].area=curr_cmpt;
					*(++child)=r[j];
				}
			}
		}while(prnt++<child);
		prnt--; //THIS LINE TOOK ME FOREVER!
		curr_cmpt++;
	}
	
	//free resources and return
	ps->narea=curr_cmpt;
	free(wkspc);
	cs_spfree(graph);
	return curr_cmpt;
}

/*
trip a collection of branches
*/
int trip_branches(PS *ps,Outage *outages)
{
	if(!(outages&&ps))return -1;
	BRANCH *branches=ps->branches,*branch_key=malloc(sizeof(BRANCH)),*branch_match=NULL;
	int i,n_outage=outages->n,from,to,nbranch=ps->nbranch,*branch_out=outages->branches;
	
	for(i=0;i<n_outage;i++)
	{
		from=get_bus_idx(ps,branch_out[2*i]);
		to=get_bus_idx(ps,branch_out[2*i+1]);
		if(to<from)SWAP_INT(from,to);
		branch_key->from=from;
		branch_key->to=to;
		branch_match=(BRANCH*)bsearch(branch_key,branches,nbranch,sizeof(BRANCH),branch_cmp);
		if(branch_match)branch_match->status=0;
	}
	
	free(branch_key);
	return 0;
}

/*
print sparse matrix to file
*/
void cs_fprint(const cs_di *A,const char *file_str)
{
	if(!A||!file_str)return;
	if(!A->x)return;
	FILE *file=fopen(file_str,"w");
	int i,j,n=A->n,*p=A->p,*r=A->i;
	double *x=A->x;

	for(i=0;i<n;i++)for(j=p[i];j<p[i+1];j++)fprintf(file,"%d %d %lg\n",r[j]+1,i+1,x[j]);
	fclose(file);
	return;
}

/*
print vector to file
*/
void vec_fprint(const double *vec,int n,const char *file_str)
{
	if(!vec||!file_str)return;	
	int i;
	FILE *file=fopen(file_str,"w");
	for(i=0;i<n;i++)fprintf(file,"%lg\n",vec[i]);
	fclose(file);
	return;
}

/*
add a branch to the outage struct
*/
void add_branch_out(Outage *outages,int from,int to,double time)
{
	//check inputs
	if(!outages)return;
	if(!outages->branches)
	{
		outages->branches=malloc(2*sizeof(int));
		if(outages->t){
			free(outages->t);
			outages->t=NULL;
		}
		outages->n=0;
		outages->nmax=1;
	}
	if(!outages->t)outages->t=malloc(outages->nmax*sizeof(double));
	
	//prep variables
	int n=outages->n,nmax=outages->nmax,*branch_out=outages->branches;
	double *t=outages->t;
	
	//if we hit nmax, double size of outage struct
	if(n==nmax)
	{
		nmax*=2;
		outages->nmax=nmax;
		branch_out=(int*)realloc(outages->branches,2*nmax*sizeof(int));
		t=(double*)realloc(outages->t,nmax*sizeof(double));
		outages->branches=branch_out;
		outages->t=t;
	}
	
	//add the input branch outage
	if(to<from)SWAP_INT(from,to);
	branch_out[2*n]=from;
	branch_out[2*n+1]=to;
	if(time<0.0)t[n]=0.0;
	else t[n]=time;
	n++;
	
	//record changes and return
	outages->n=n;
	return;
}

/*
free outage memeory
*/
void free_outages(Outage *outages)
{
	if(!outages)return;
	if(outages->branches)free(outages->branches);
	if(outages->t)free(outages->t);
	free(outages);
	return;
}

#include "ps.h"
#include "dcsimsep.h"

/*
print command line flag usage
*/
static void Usage(void)
{
	printf("Usage:\n");
	printf(" -f <mesh_file_name>\n");
	exit(8);
}

/*
test program
*/
int main(int argc,char** argv)
{	
	//prep variables
	char ps_file[256]={0},ps_name_f[256]={0},bo_file[256]={0},
	*results_file="test/dcsimsep_results.txt";
	PS *ps;
	BRANCH *branches;
	Outage *outages;
	int i,ncmpt,nbus,*bo_pairs;
	cs_di **bbuses;
	double *vang;
	FILE *file;
	
	//parse command line inputs
	while((argc>1)&&(argv[1][0]=='-'))
	{		
		switch (argv[1][1])
		{
			case 'f':
				strcpy(ps_file,&argv[2][0]);
				argv+=2;
				argc-=2;
				break;
			case 'b':
				strcpy(bo_file,&argv[2][0]);
				argv+=2;
				argc-=2;
				break;
			case 'h':
				Usage();
			default:
				printf("Unknown flag: %s\n", argv[1]);
				Usage();
		}
	}
	
	//load up ps
	ps=load_ps(ps_file);
	if(!ps)exit(8);
	//vang=malloc(ps->nbus*sizeof(double));
	
	//load blackout pairs
	outages=malloc(sizeof(Outage));
	outages->branches=NULL;
	outages->t=NULL;
	add_branch_out(outages,4,16,0.0);
	add_branch_out(outages,11,17,0.0);
	
	/*
	trip_branches(ps,outages);
	redispatch(ps);
	dcpf(ps);
	sprintf(ps_name_f,"test/case%d_vang.txt",ps->nbus);
	for(i=0;i<ps->nbus;i++)vang[i]=ps->buses[i].vang;
	vec_fprint(vang,ps->nbus,ps_name_f);
	*/
	
	//run dcsimsep and write results to file
	dcsimsep(ps,outages);
	file=fopen(results_file,"w");
	fprint_outages(file,outages);
	fclose(file);
	
	/*
	//print banner
	printf("\n///////////////////////////////////////////////////////\n");
	printf("RUNNING FILE %s...\n",ps_file);
	printf("///////////////////////////////////////////////////////\n");
	
	//print some output
	printf("\nBus Locations:\n\n");
	for(i=0;i<ps->nbus;i++){
		printf("Bus %3d: loc %2d, ",ps->buses[i].id,ps->buses[i].area);
		if(!((i+1)%7))printf("\n");
	}
	printf("\n");
	
	//print out ps
	//ps_print(ps,1);
	
	//run dcpf
	dcpf(ps);
	printf("\nDCPF Results:\n\n");
	for(i=0;i<ps->nbus;i++){
		printf("Bus %3d Ang: %lg, ",ps->buses[i].id,ps->buses[i].vang);
		if(!((i+1)%5))printf("\n");
	}
	printf("\n");
	
	//print bbus matrices to file then free them
	bbuses=get_bbus(ps);
	for(i=0;i<ps->narea;i++){
		printf("\n");
		cs_print(bbuses[i],1);
		sprintf(ps_name_f,"test/case%d_matrix%d.txt",ps->nbus,i);
		cs_fprint(bbuses[i],ps_name_f);
		cs_spfree(bbuses[i]);
	}
	
	//write out bus thetas
	for(i=0;i<ps->nbus;i++)vang[i]=ps->buses[i].vang;
	sprintf(ps_name_f,"test/case%d_vang.txt",ps->nbus);
	vec_fprint(vang,ps->nbus,ps_name_f);
	*/
	
	//clean up and return
	ps_free(ps);
	free_outages(outages);
	return 0;
}

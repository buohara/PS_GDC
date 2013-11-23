#include "ps.h"
#include "acsimsep.h"

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
	*results_file="test/acsimsep_results.txt";
	PS *ps;
	cs_ci** ybus;
	int i,narea;
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
	ybus=get_ybus(ps);
	narea=ps->narea;
	for(i=0;i<narea;i++){cs_ci_print(ybus[i],1);cs_ci_spfree(ybus[i]);}
	free(ybus);
	ps_free(ps);
	return 0;
}

#include "varint.h"

//print command line flag usage
void Usage(void){
	printf("Usage:\n");
	printf(" -f <mesh_file_name>\n");
	printf(" -d <name>\n");
	exit(8);
}

//main
int main(int argc, char** argv){
	
	//define variables
	mesh* msh=malloc(sizeof(mesh));
	kchn* chn=NULL;
	bdry* bdry=NULL;
	cs_di* mw_mat=NULL;
	float* bctrs=NULL;
	int i;
	
	//Parse command line arguments
	char mesh_file[256];
	if(!(argc%2))Usage();
	while ((argc>1)&&(argv[1][0]=='-')){		
		switch (argv[1][1]){
			case 'f':
				strcpy(mesh_file,&argv[2][0]);
				break;
			case 'd':
				printf("%s\n",&argv[2][0]);
				break;
			case 'h':
				Usage();
			default:
				printf("Unknown flag: %s\n", argv[1]);
				Usage();
		}
		argv+=2;
		argc-=2;
	}
	
	//load 2-chain from file
	chn=kchn_load_2chn(mesh_file,msh,TRUE);
	
	//compute the boundary and face-edge adjacency matrix
	bdry=kchn_bdry(chn);
	
	//compute and print the maxwell matrix
	mw_mat=maxwell_matrix(chn,bdry);
	cs_print(mw_mat,1);
	
	bctrs=barycenters(chn,msh);
	for(i=0;i<1000;i++)printf("%g ",bctrs[i]);
	
	//free up resources
	kchn_free(chn);
	kchn_free_bdry(bdry);
	cs_spfree(mw_mat);
	FreeMesh(msh);
	free(bctrs);
		
	//return
	return 0;
}

#include "ps.h"

/*
load pairs of branch failures from file
*/
Outage* load_outages(const char* file_str)
{
	//check input and prep variables
	if(!file_str)return NULL;
	FILE *file=fopen(file_str,"r");
	int i,n,f1,t1,f2,t2;
	Outage *outages=malloc(sizeof(Outage));
	
	//read in number of outage pairs and scan them in
	fscanf(file,"%d\n",&n);
	for(i=0;i<1;i++)
	{
		fscanf(file,"%d %d %d %d\n",&f1,&t1,&f2,&t2);
		add_branch_out(outages,f1,t1,0.0);
		add_branch_out(outages,f2,t2,0.0);
	}
	
	//close file and return
	fclose(file);
	return outages;
};

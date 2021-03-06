#include "ps.h"

/*
print branch failures to file
*/
int fprint_outages(FILE *file,Outage *outages)
{
	//check inputs and prep variables
	if(!(file&&outages))return -1;
	int i,n=outages->n,*branches=outages->branches;
	double *t=outages->t;
	
	//print data to file
	fprintf(file,"%d\n",n);
	for(i=0;i<n;i++)
	{
		fprintf(file,"%d %d %lg\n",branches[2*i],branches[2*i+1],t[i]);
	}
		
	//return
	return 0;
}

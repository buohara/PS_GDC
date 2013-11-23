#include "simplex.h"
#include <stdio.h>

int main(int argc, char**argv){

	kchn* ch2=kchn_alloc(3,5,0);
	bdry* bdry;

	unsigned int idcs[5][5]={
		{3,2,1,4,0},
		{4,3,2,5,0},
		{5,4,3,6,1},
		{7,2,3,6,1},
		{5,8,4,6,2}
	};

	kchn_entry(ch2,idcs[1]);
	kchn_entry(ch2,idcs[0]);
	kchn_entry(ch2,idcs[2]);
	kchn_entry(ch2,idcs[4]);
	kchn_entry(ch2,idcs[3]);

	printf("Raw Chains:\n\n");

	kchn_print(ch2,0);
	kchn_finalize(ch2);

	printf("Finalized Chains:\n\n");

	kchn_print(ch2,0);

	printf("Boundary:\n\n");	

	bdry=kchn_bdry(ch2);
	kchn_print(bdry->dchn,0);
	cs_print(bdry->adj,0);
	
	kchn_free(ch2);
	kchn_free_bdry(bdry);

	return 0;
}

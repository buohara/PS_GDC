#ifndef SIMPLEX_H
#define SIMPLEX_H

#define SWAP(a,b) do{a^=b;b^=a;a^=b;}while(0)

#include <stdio.h>
#include <stdlib.h>
#include <suitesparse/cs.h>

/*
simplicial k-chain structure
*/
typedef struct{
	int k;
	int ele_sz;
	int smpmax;
	int nsmp;
	int *smps;
}kchn;

typedef struct{
	kchn* dchn;
	cs_di* adj;
}bdry;

/*
container of values to render a mesh.
*/

typedef struct{
	int nv;
	float *verts;
}mesh;

/*
allocate memory for a k-chain
*/
kchn* kchn_alloc(int k,int ele_sz,int smpmax);

//free a k-chain
void kchn_free(kchn* chn);

/*
finalize a k-chain. this just releases any extra memory not being used.
if kchn_entry is called to add new entries, kchn_finalize must also be
called again.
*/
void kchn_finalize(kchn* chn);

/*
sort a k-chain using the indices of its simplices
*/
void kchn_sort(kchn* chn);

/*
add an entry to a k-chain
*/
int kchn_entry(kchn *chn,int *ele);

/*
Compute the boundary of a k-chain and return its
k-simplex to (k-1)-simplex sparse adjacency matrix.
*/
bdry* kchn_bdry(const kchn* chn);

/*
Free boundary memory.
*/

void kchn_free_bdry(bdry* bdry);

/*
print chain to console
*/
void kchn_print(const kchn *chn,int brief);

/*
obtain the boundary simplices of a given simplex and the relative
orientation of these simplices
void kchn_smp_bdry(const int *smp,int* bdry,int k);
*/

/*
populate a 2-chain from file. optionally, keep vertex data and vertex indices
for rendering.
*/
kchn* kchn_load_2chn(const char* file_nm,mesh* msh,int keep_mesh);

/*
free mesh memory
*/
void FreeMesh(mesh* msh);

#endif

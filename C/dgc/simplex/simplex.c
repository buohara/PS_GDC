/*
includes
*/
#include "simplex.h"
#include <math.h>

/*
k_g is a global k-chn order. k_g is set before qsort is called on 
a k-chain so that the kchn_cmp comparison function knows how many vertex indices 
to be compared.
*/
int k_g;

/*
allocate a k-chain.
*/
kchn* kchn_alloc(int k,int ele_sz,int smpmax){
	if(ele_sz<k+1){return NULL;}	
	kchn* chn=malloc(sizeof(kchn));
	if(!chn)return NULL;
	chn->k=k;
	chn->ele_sz=ele_sz;
	chn->nsmp=0;
	smpmax>0?(chn->smpmax=smpmax):(chn->smpmax=1);
	chn->smps=malloc(smpmax*ele_sz*sizeof(int));
	if(!(chn->smps)){free(chn);return NULL;}
	return chn;
};

/*
free a k-chain's memory
*/
void kchn_free(kchn* chn){
	if(!chn) return;
	if(chn->smps)free(chn->smps);
	free(chn);
	return;
}

/*
reorder a simplex's vertex indices into ascending order and return the sign
of the resulting permutation. kchn_perm uses a simple insertion sort 
that keeps track of swaps.
*/
inline static int kchn_perm(int* list,int len){
	int i,j,sgn=0;
	if(len<2)return 1;
	for(i=1;i<len;i++){
		j=i;
		while(list[j]<list[j-1]&&j>0){SWAP(list[j],list[j-1]);j--;sgn++;}
	}
	return sgn%2?-1:1;
}

/*
comparison function for simplices used by qsort. this function uses k_g
to determine how many vertex indices should be compared.
*/
inline static int kchn_cmp(const void* first,const void* second){
	int *smp1=(int*)first,*smp2=(int*)second,i;
	for(i=0;i<k_g;i++){
		if(smp1[i]!=smp2[i])return smp1[i]-smp2[i];
	}
	return smp1[k_g]-smp2[k_g];
}

/*
k-chain sort function. set the global k_g variable, then pass an array 
of simplices to qsort for sorting.
*/
inline void kchn_sort(kchn* chn){
	int nsmp,*smps,ele_sz;		
	if(!chn)return;
	k_g=chn->k;ele_sz=chn->ele_sz;nsmp=chn->nsmp;smps=chn->smps;
	qsort((void*)smps,nsmp,ele_sz*sizeof(int),kchn_cmp);
}

/*
add a simplex to a k-chain. if the maximum number of simplices for the chain is reached,
the max is dynamically doubled.
*/
int kchn_entry(kchn* chn,int *ele){
	if(!chn||!ele)return -1;
	int k=chn->k,smpmax=chn->smpmax,ele_sz=chn->ele_sz,nsmp=chn->nsmp,*smps,*smps_tmp,i;	
	if(smpmax==nsmp){
		smps_tmp=realloc(chn->smps,2*ele_sz*smpmax*sizeof(int));
		if(!smps_tmp)return -1;
		chn->smps=smps_tmp;	
		chn->smpmax*=2;
	}
	kchn_perm(ele,k+1);
	smps=chn->smps;
	for(i=0;i<ele_sz;i++)smps[nsmp*ele_sz+i]=ele[i];
	chn->nsmp++; 
	return 0;
}

/*
kchn_finalize is called after all simplices have been added to a chain.
kchn_finalize sorts the simplices, merges duplicates,
and frees any excess memory. simplices can still be added to the chain, but
kchn_finalize should be called again after entries have been added.
*/
void kchn_finalize(kchn* chn){

	//local variables and input check	
	int k,nsmp,unsmp=1,p,i,j,match,*smps,ele_sz;	
	if(!chn)return;
	if(chn->nsmp==1)return;

	//sort the chain entries
	kchn_sort(chn);

	//merge duplicates
	k=chn->k;ele_sz=chn->ele_sz;nsmp=chn->nsmp;smps=chn->smps;p=1;
	for(i=1;i<nsmp;i++){
		match=1;
		for(j=0;j<(k+1);j++)match&=(smps[i*ele_sz+j]==smps[(i-1)*ele_sz+j]);
		if(!match){
			for(j=0;j<ele_sz;j++){
				smps[ele_sz*p+j]=smps[i*ele_sz+j];
			}
			p++;
		}
	}
		
	//free excess memory and return
	chn->smpmax=chn->nsmp=p;
	chn->smps=realloc(chn->smps,ele_sz*p*sizeof(int));
}

/*
print a k-chain. if brief is true, only the first 20 simplices are printed.
*/
void kchn_print(const kchn* chn,int brief){
	if(!chn)return;
	int k=chn->k,ele_sz=chn->ele_sz,smpmax=chn->smpmax,nsmp=chn->nsmp,*smps=chn->smps,i,j;
	printf("Simplicial %d-complex: %d simplices\n",k,nsmp);
	for(i=0;i<nsmp;i++){
		for(j=0;j<ele_sz;j++)j?printf(",%d",smps[i*ele_sz+j]):printf("(%d",smps[i*ele_sz]);
		printf(")\n");
		if(brief&&i>20){printf("...\n");return;}
	}
	printf("\n");
};

/*
Compute the boundary of a k-chain. It is assumed that the input chain has k+2 values per entry:
k+1 vertex indices, and 1 value for its physical group. Returns a (k-1)-chain with entries as follows:

(v1,...,vk,phygrp),

where the vi are vertex indicies and physgrp is the physical group to which the simplex belongs, as
well as a k-simplex to (k-1)-simplex sparse adjacency matrix with entries 1 and -1 depending on
the relative orientation of a simplex and its boundary simplex.
*/
bdry* kchn_bdry(const kchn* chn){

	//check input
	if(!chn)return NULL;

	//initialize variables
	int i,j,l,k=chn->k,ele_sz=chn->ele_sz,*smps=chn->smps,*dsmps,*dsmp_entry,nsmp=chn->nsmp,ndsmp,match,d_ele_sz;
	bdry* bdry;
	kchn* dchn;
	cs_di *adj,*adj_tmp;
	if(k<1||ele_sz<(k+2))return NULL;
	bdry=malloc(sizeof(bdry));
	d_ele_sz=k+2;
	dchn=kchn_alloc(k-1,d_ele_sz,0);
	adj_tmp=cs_spalloc(0,0,0,1,1);
	dsmp_entry=malloc(d_ele_sz*sizeof(int));
	
	/*
	first, loop through the input simplices generating a list of boundary simplices. each 
	boundary simplex has k+2 values: k vertex indices, 1 adjacent k-simplex, and 1 value
	for its physical group.
	*/
	for(i=0;i<nsmp;i++){
		for(j=0;j<(k+1);j++){
			for(l=0;l<k;l++)dsmp_entry[l]=smps[i*ele_sz+l+(j<=l)];
			dsmp_entry[k]=smps[i*ele_sz+(k+1)];
			dsmp_entry[k+1]=(j%2?(-(i+1)):(i+1));
			kchn_entry(dchn,dsmp_entry);
		}		
	}
	
	//sort the edges by beginning vertex and ending vertex
	ndsmp=dchn->nsmp;k_g=k-1;dsmps=dchn->smps;
	qsort((void*)dsmps,ndsmp,d_ele_sz*sizeof(int),kchn_cmp);
	
	/*
	generate the sparse adjacency matrix. since the boundary simplices are sorted, we
	create a running list of unique boundary simplex indices by doing a running comparison
	of a simplex and its predecessor in the list. we start at index l=0, and as simplices
	with new vertex indices are encountered, we increment l. 
	*/
	l=0;
	cs_entry(adj_tmp,abs(dsmps[k+1])-1,0,(dsmps[k+1]>0?1:-1));
	for(i=1;i<ndsmp;i++){
		match=1;
		for(j=0;j<k;j++)match&=(dsmps[i*d_ele_sz+j]==dsmps[(i-1)*d_ele_sz+j]);
		if(!match)l++;
		cs_entry(adj_tmp,abs(dsmps[i*d_ele_sz+(k+1)])-1,l,(dsmps[i*d_ele_sz+(k+1)]>0?1:-1));
	}

	/*
	Now finalize the boundary chain. Since we intend to return a chain that does not
	have entries with adjacent parent simplex information, we can't use kchn_finalize.
	instead, we'll do the merge manually here.
	*/
	l=1;
	for(i=1;i<ndsmp;i++){
		match=1;
		for(j=0;j<k;j++)match&=(dsmps[i*d_ele_sz+j]==dsmps[(l-1)*(d_ele_sz-1)+j]);
		if(!match){
			for(j=0;j<(d_ele_sz-1);j++){
				dsmps[(d_ele_sz-1)*l+j]=dsmps[i*d_ele_sz+j];
			}
			l++;
		}
	}
	
	//compress the adjacency matrix
	adj=cs_compress(adj_tmp);	
		
	//free excess memory and set boundary chain parameters.
	dchn->smpmax=dchn->nsmp=l;dchn->ele_sz=d_ele_sz-1;
	dchn->smps=realloc(dchn->smps,(d_ele_sz-1)*l*sizeof(int));

	//free memory and return
	free(dsmp_entry);
	cs_spfree(adj_tmp);
	bdry->dchn=dchn;
	bdry->adj=adj;
	return bdry;
}

/*
Free a k-chain boundary
*/
void kchn_free_bdry(bdry* bdry){
	if(bdry->dchn)kchn_free(bdry->dchn);
	if(bdry->adj)cs_spfree(bdry->adj);
	free(bdry);
	return;
}

/*
DEPRECATED

create a face-face adjacency matrix from a list of input faces. this function first computes a
matrix D of edge-face adjacencies using a discrete boundary operator. the matrix (D^T)D is
then computed and returned as a sparse matrix. it is assumed that the input chain
has already been finalized.

cs_di* kchn_fadj(const kchn* chn){

	//check input
	if(!chn)return NULL;

	//initialize variables
	int i,j,l,k=chn->k,ele_sz=chn->ele_sz,*smps=chn->smps,*dsmps,*dsmp_entry,nsmp=chn->nsmp,ndsmp,match;
	cs_di *d,*d2,*df;
	kchn* dchn;
	if(k!=2)return NULL;
	dchn=kchn_alloc(k-1,k+1,0);
	dsmp_entry=malloc((k+1)*sizeof(int));
	
	
	first, loop through the input faces generating a list of edges. each edge has 3 values:
	a beginning vertex, an end vertex, and an integer indicating the index of a face it's 
	adjacent to.
	
	for(i=0;i<nsmp;i++){
		for(j=0;j<(k+1);j++){
			for(l=0;l<k;l++)dsmp_entry[l]=smps[i*ele_sz+l+(j<=l)];
			dsmp_entry[k]=i;
			kchn_entry(dchn,dsmp_entry);
		}		
	}
	
	//sort the edges by beginning vertex and ending vertex
	ndsmp=dchn->nsmp;k_g=k-1;l=0;dsmps=dchn->smps;
	qsort((void*)dchn->smps,ndsmp,(k+1)*sizeof(int),kchn_cmp);
	
	
	generate a running list of unique edge indices. since the edges are sorted,
	we compare a given edge to the previous edge in the list. we start with index number 0,
	and as new edges are encountred, we increment the index number. once edge i-1 has been compared to
	edge i, we don't need to know its vertex indices anymore, so we can overwrite one of
	these indices with its edge index.
	
	for(i=1;i<ndsmp;i++){
		match=1;
		for(j=0;j<k;j++)match&=(dsmps[i*(k+1)+j]==dsmps[(i-1)*(k+1)+j]);
		dsmps[(i-1)*(k+1)+(k-1)]=l;
		if(!match)l++;
	}

	//build the edge-face adjacency matrix D, get its transpose, and compute (D^T)D
	d=cs_spalloc(0,0,0,0,1);
	for(i=0;i<ndsmp;i++)cs_entry(d,dsmps[i*(k+1)+(k-1)],abs(dsmps[i*(k+1)+k]),0);
	d2=cs_compress(d);
	cs_spfree(d);
	d=cs_transpose(d2,0);
	df=cs_multiply(d,d2);

	//free memory and return
	free(dsmp_entry);
	kchn_free(dchn);
	cs_spfree(d);
	cs_spfree(d2);
	return df;
}
*/

/*
DEPRECATED

obtain the boundary simplices of a given simplex and the relative
orientation of these simplices. it is assumed that the input simplex's
indices are already in ascending order.

inline void kchn_smp_bdry(const int *smp,int* bdry,int k){
	if(!smp||!bdry)return;
	int j,l;
	for(j=0;j<(k+1);j++){
		for(l=0;l<k;l++)bdry[l+j*k]=smp[l+(j<=l)];
	}
	return;
}
*/

/*
populate a 2-chain from gmsh .msh file. the file begins with the x,y,z coordinates
then vertex indices of elements in the mesh. we read vertices into memory first. next,
a list of mesh elements (surface faces, tetrahedra, etc) are listed along with the
vertex indices that make up the element, along with any element properties (e.g.,
materials) that might be of interest. we use these elements to populate our 2-chain.
*/
kchn* kchn_load_2chn(const char* file_nm,mesh* msh,int keep_mesh){
	
	//check inputs and initialize variables
	if(!file_nm)return NULL;
	FILE *file;
	char curr_line[256];
	int nd_ct,i,j,l,idx,ele_ct,ele_tp,attrib_ct,ele_grp,bdry[16],entry[4],face_max=1;
	int* fidcs_tmp;
	float x,y,z;
	kchn* chn;

	//try to open mesh file
	file=fopen(file_nm,"r");
	if(!file) return NULL;

	//skip through the header
	fgets(curr_line,256,file);
	fgets(curr_line,256,file);
	fgets(curr_line,256,file);

	//we should be at the nodes/vertex section of the file now. check this.
	if(!strcmp(fgets(curr_line,256,file),"$Nodes"))return NULL;

	//get the number of vertices
	if(!fgets(curr_line,256,file)) return NULL;
	sscanf(curr_line,"%d\n",&nd_ct);

	/*
	if we're keeping the mesh, allocate nd_ct (x,y,z,w) vertices. while we're at it
	initialize the mesh indices
	*/
	if(keep_mesh){
		msh->verts=malloc(3*nd_ct*sizeof(float));
		if(!msh->verts){FreeMesh(msh);return NULL;}
		msh->nv=nd_ct;
	}
	
	//populate the verts if needed
	for(i=0;i<nd_ct;i++){
		if(!fgets(curr_line,256,file)){FreeMesh(msh);return NULL;}
		if(keep_mesh){
			sscanf(curr_line,"%d %f %f %f\n",&idx,&x,&y,&z);
			msh->verts[i*3]=x;
			msh->verts[i*3+1]=y;
			msh->verts[i*3+2]=z;
		}
	}

	//skip some lines
	fgets(curr_line,256,file);
	fgets(curr_line,256,file);

	if(!strcmp(fgets(curr_line,256,file),"$Elements")){FreeMesh(msh);return NULL;}
	sscanf(curr_line,"%d\n",&ele_ct);

	//allocate an empty 2-chain to begin populating
	chn=kchn_alloc(2,4,0);
	
	/*
	loop through the elements section of the file and populate the vertex indices of
	the elements.
	*/
	for(i=0;i<ele_ct;i++){
		
		//read a line and get the type of the element
		if(!fgets(curr_line,256,file)){FreeMesh(msh);kchn_free(chn);return NULL;}
		sscanf(curr_line,"%d %d",&idx,&ele_tp);

		//process the element as needed and add it to the chain
		switch(ele_tp){
			
			/*
			case 2 is a triangle. no boundary processing is necessary, so just
			add it to our 2-chain. if we're keeping the mesh, add these indices
			to msh->indices.
			*/
			case 2:
			
				//scan in the face and add it to the kchn
				sscanf(curr_line,"%d %d %d %d %d %d %d %d",&idx,&ele_tp,&attrib_ct,entry+3,&ele_grp,entry,entry+1,entry+2);
				kchn_entry(chn,entry);
				break;
			
			/*
			case 4 is a tet. after reading in its indices, compute its
			4 boundary faces. add these to the face chain. if we're keeping the mesh,
			add the face indices to msh->indices.
			*/
			case 4:
			
				//scan in the face and add it to the kchn
				sscanf(curr_line,"%d %d %d %d %d %d %d %d %d",&idx,&ele_tp,&attrib_ct,entry+4,&ele_grp,entry,entry+1,entry+2,entry+3);
				for(j=0;j<4;j++){
					for(l=0;l<3;l++)bdry[l+j*4]=entry[l+(j<=l)];
					bdry[j*4+3]=entry[4];
				}
				kchn_entry(chn,bdry);
				kchn_entry(chn,bdry+4);
				kchn_entry(chn,bdry+8);
				kchn_entry(chn,bdry+12);
				break;
			
			/*
			Ignore any other cases for now.
			*/
			default:
				continue;
		}
	}

	//finalize the face chain.
	kchn_finalize(chn);
	
	//close mesh file
	fclose(file);	

	//return
	return chn;
}

/*
free mesh memory
*/
void FreeMesh(mesh* msh){
	if(!msh) return;
	if(msh->verts) free(msh->verts);
	free(msh);
	return;
}

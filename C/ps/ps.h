/*

file: ps.h
author: Ben O'Hara
about: declarations of a power system data structures and functions for loading ps data from
file.

*/

/*
frontmatter
*/
#ifndef PS_H
#define PS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <suitesparse/cs.h>

#ifdef __cplusplus
extern "C"{
#endif

/*
some macro metaprogramming trickery. this saves a lot of grief if you
change/add/remove items from ps and its constituent data structures.
*/

#define MSG printf("Here\n");

#define BUS_VARS \
	X(int,id,"%d") \
	X(int,type,"%d") \
	X(double,pd,"%lg") \
	X(double,qd,"%lg") \
	X(double,gs,"%lg") \
	X(double,bs,"%lg") \
	X(int,area,"%d") \
	X(double,vmag,"%lg") \
	X(double,vang,"%lg") \
	X(double,base_kv,"%lg") \
	X(int,zone,"%d") \
	X(double,vmax,"%lg") \
	X(double,vmin,"%lg") \
	X(double,lam_p,"%lg") \
	X(double,lam_q,"%lg") \
	X(double,mu_vx,"%lg") \
	X(double,mu_vn,"%lg") \
	X(double,loc_x,"%lg") \
	X(double,loc_y,"%lg")

#define BRANCH_VARS \
	X(int,from,"%d") \
	X(int,to,"%d") \
	X(double,r,"%lg") \
	X(double,x,"%lg") \
	X(double,bc,"%lg") \
	X(double,rate_a,"%lg") \
	X(double,rate_b,"%lg") \
	X(double,rate_c,"%lg") \
	X(int,tap,"%d") \
	X(double,shift,"%lg") \
	X(int,status,"%d") \
	Y(double,threshold,"") \
	Y(double,state_a,"") \
	Y(double,imag,"") \
	Y(double,b,"")
	
#define GEN_VARS \
	X(int,bus,"%d") \
	X(double,pg,"%lg") \
	X(double,qg,"%lg") \
	X(double,qmax,"%lg") \
	X(double,qmin,"%lg") \
	X(double,vsp,"%lg") \
	X(double,mbase,"%lg") \
	X(int,status,"%d") \
	X(double,pmax,"%lg") \
	X(double,pmin,"%lg")
	/*
	X(double,mu_px,"%lg") \
	X(double,mu_pn,"%lg") \
	X(double,mu_qx,"%lg") \
	X(double,mu_qn,"%lg") \
	X(int,type,"%d") \
	X(double,cost,"%lg")
	*/
	
#define SHUNT_VARS \
	X(int,bus,"%d") \
	X(double,p,"%lg") \
	X(double,q,"%lg") \
	X(double,frac_z,"%lg") \
	X(double,frac_s,"%lg") \
	Y(double,frac_d,"")
	/*
	X(int,status,"%d") \
	X(int,type,"%d") \
	X(double,value,"%lg")	
	*/
	
/*
generate ps data structures
*/
#define X(type,name,fmt) type name;
#define Y(type,name,fmt) type name;
#define MAKE_STRUCT(ELEMENT_T) typedef struct{ \
	ELEMENT_T##_VARS \
}ELEMENT_T;
MAKE_STRUCT(BUS)
MAKE_STRUCT(BRANCH)
MAKE_STRUCT(GEN)
MAKE_STRUCT(SHUNT)
//MAKE_STRUCT(AREA)
#undef X
#undef Y

#define X(type,name,fmt) fmt" "
#define Y(type,name,fmt)
#define FMT_STR(ELEMENT_T) static const char* ELEMENT_T##_STR= \
	ELEMENT_T##_VARS; 
FMT_STR(BUS)
FMT_STR(BRANCH)
FMT_STR(GEN)
FMT_STR(SHUNT)
//FMT_STR(AREA)
#undef X

//#define X(type,name,fmt) printf(fmt" ",ptr->name);
#define X(type,name,fmt) ,ptr->name
#define MAKE_PRINT_FUNC(ELEMENT_T) \
__inline static void PRINT_##ELEMENT_T(ELEMENT_T *ptr) \
{ \
	printf(ELEMENT_T##_STR \
	ELEMENT_T##_VARS \
	); \
	printf("\n"); \
}
MAKE_PRINT_FUNC(BUS)
MAKE_PRINT_FUNC(BRANCH)
MAKE_PRINT_FUNC(GEN)
MAKE_PRINT_FUNC(SHUNT)
//MAKE_PRINT_FUNC(AREA)
#undef X

#define X(type,name,fmt) ,&(ptr->name)
#define MAKE_SCAN_FUNC(ELEMENT_T) \
__inline static void SCAN_##ELEMENT_T(const char* str,ELEMENT_T *ptr) \
{ \
	sscanf(str,ELEMENT_T##_STR \
	ELEMENT_T##_VARS \
	); \
}
MAKE_SCAN_FUNC(BUS)
MAKE_SCAN_FUNC(BRANCH)
MAKE_SCAN_FUNC(GEN)
MAKE_SCAN_FUNC(SHUNT)
//MAKE_PRINT_FUNC(AREA)
#undef X

//integer swap
#define SWAP_INT(x,y){x=x+y;y=x-y;x=x-y;}

/*
power system data structure.
*/
typedef struct
{
	double base_mva;
	int nbus;
	int nbranch;
	int ngen;
	int nshunt;
	int narea;	
	BUS *buses;
	BRANCH *branches;
	GEN *gens;
	SHUNT *shunts;
	//AREA *areas;
	int* bus_sub_idcs;
	int* area_sizes;
	int* ref_buses;
}PS;

/*
struct of line outages
*/
typedef struct
{
	int n;
	int nmax;
	int *branches;
	double *t;
}Outage;

/*
bus comparison by ID.
*/
__inline static int bus_cmp(const void *bus1_in,const void *bus2_in)
{
	BUS *bus1=(BUS*)bus1_in;
	BUS *bus2=(BUS*)bus2_in;
	return bus1->id-bus2->id;
}

/*
branch comparison by to/from.
*/
__inline static int branch_cmp(const void *branch1_in,const void *branch2_in)
{
	BRANCH *branch1=(BRANCH*)branch1_in;
	BRANCH *branch2=(BRANCH*)branch2_in;
	if(branch1->from!=branch2->from)return branch1->from-branch2->from;
	return branch1->to-branch2->to;
}

/*
get a bus's index (i.e., its row/column in Ybus) from its
ID
*/
int get_bus_idx(const PS *ps,int id);

/*
load power system data from *.m file.
*/
PS* load_ps(const char *file);

/*
free ps memory
*/
void ps_free(PS *ps);

/*
print ps to console
*/
void ps_print(const PS *ps,int brief);

/*
traverse the ps and find all its connected subcomponents.
*/
int find_subgraphs(PS *ps);

/*
print sparse matrix to file
*/
void cs_fprint(const cs_di *A,const char *file_str);

/*
print vector to file
*/
void vec_fprint(const double *vec,int n,const char *file_str);

/*
trip branch between from and to
*/
int trip_branches(PS *ps,Outage *outages);

/*
add a branch to the outage struct
*/
void add_branch_out(Outage *outages,int from,int to,double time);

/*
free outage memeory
*/
void free_outages(Outage *outages);

/*
load pairs of branch failures from file
*/
Outage* load_outages(const char* file_str);

/*
print branch failures to file
*/
int fprint_outages(FILE *file,Outage *outages);

#ifdef __cplusplus
}
#endif

#endif

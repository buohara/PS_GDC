#include "varint.h"

float mats[2][2]={
	{0.4f,0.4f},
	{1.0f,1.0f}
};

static cs *cs_multiply_sgn(const cs *A,const cs *B);
static int cs_scatter_sgn(const cs *A,int j,int n,double beta,int *w,double *x,int mark,cs *C,int nz);

/*
Given a 2-chain, its boundary, and material properties, compute the
maxwell update matrix 2*I-d*eps^(-1)*d^T*mu^(-1).
*/
cs_di* maxwell_matrix(const kchn* chn,const bdry* bdry){
	
	//check inputs
	if(!chn||!bdry)return NULL;
	if(!(bdry->dchn)||!(bdry->adj))return NULL;
	
	//declare variables and populate d, d^T, and I
	int i,j,ele_sz=chn->ele_sz,d_ele_sz=bdry->dchn->ele_sz,m=bdry->adj->m,n=bdry->adj->n,*dp=NULL,*d_tp=NULL;
	kchn* dchn=bdry->dchn;
	double eps,mu,*dx=NULL,*d_tx=NULL;
	cs_di *d=NULL,*d_t=NULL,*Id=NULL,*Id_tmp=NULL,*maxwell_tmp=NULL,*maxwell_f=NULL;
	d_t=cs_transpose(bdry->adj,1);
	Id_tmp=cs_spalloc(m,m,m,1,1);
	d=cs_transpose(d_t,1);
	
	//grab matrix pointers
	dp=d->p;dx=d->x;d_tp=d_t->p;d_tx=d_t->x;
	
	//build the 2I matrix
	for(i=0;i<m;i++)cs_entry(Id_tmp,i,i,1.0);
	Id=cs_compress(Id_tmp);
	cs_spfree(Id_tmp);
	
	//scale the columns of d so that d<=d*eps
	for(i=0;i<n;i++){
		eps=1.0/(mats[dchn->smps[d_ele_sz*(i+1)-1]][EPSILON]);
		for(j=dp[i];j<dp[i+1];j++){
			dx[j]*=eps;
		}
	}
	
	//scale the columns of d^T so that d^T<=d^T*mu
	for(i=0;i<m;i++){
		mu=1.0/(mats[chn->smps[ele_sz*(i+1)-1]][MU]);
		for(j=d_tp[i];j<d_tp[i+1];j++){
			d_tx[j]*=mu;
		}
	}
	
	//compute the first part of maxwell, d*eps*d^T*mu
	maxwell_tmp=cs_multiply_sgn(d,d_t);
	
	//free up some memory that we don't need
	cs_spfree(d);cs_spfree(d_t);
	
	//compute final maxwell
	maxwell_f=cs_add(Id,maxwell_tmp,2.0,-0.05);
	
	//free memory and return
	cs_spfree(Id);cs_spfree(maxwell_tmp);
	return maxwell_f;
}

/*
Given a list of vertices and a k-chain, compute the barycenters of
the k-chain simplices.
*/
float* barycenters(const kchn* chn, const mesh* msh){
	
	//check inputs and setup variables	
	if(!chn||!msh)return NULL;
	int i,j,l,k=chn->k,*smps=chn->smps,ele_sz=chn->ele_sz,nsmp=chn->nsmp;
	float *bctrs=malloc((k+1)*nsmp*sizeof(float)),kinv=1.f/(k+1),*verts=msh->verts;
	
	//loop through the simplices generating barycenters.
	for(i=0;i<nsmp;i++){	
		for(l=0;l<3;l++){
			bctrs[i*3+l]=0.f;
			for(j=0;j<(k+1);j++)bctrs[i*3+l]+=kinv*verts[3*(smps[i*ele_sz+j]-1)+l];
		}
	}
	
	//return
	return bctrs;
}

/*

*/
kchn* kchn_idcs(const kchn* chn){
	
	//check inputs and initialize variables
	if(!chn)return NULL;
	int i,j,k=chn->k,nsmp=chn->nsmp,ele_sz=chn->ele_sz,*smps=chn->smps,*ismps=NULL;
	kchn *ichn=kchn_alloc(k,(k+1),nsmp);
	ichn->nsmp=nsmp;
	ismps=ichn->smps;
	
	//extract and return the mesh indices
	for(i=0;i<nsmp;i++){
		for(j=0;j<(k+1);j++)ismps[i*(k+1)+j]=smps[i*ele_sz+j]-1;
	}
	
	//return the mesh indices
	return ichn;
}

/*
This is a nearly verbatim copy of Tim Davis's cs_multiply function. The only difference is
the call to cs_scatter has been changed to cs_scatter_sgn.
*/
cs *cs_multiply_sgn(const cs *A,const cs *B)
{
    int p, j, nz = 0, anz, *Cp, *Ci, *Bp, m, n, bnz, *w, values, *Bi ;
    double *x, *Bx, *Cx ;
    cs *C ;
    if (!CS_CSC (A) || !CS_CSC (B)) return (NULL) ;      /* check inputs */
    if (A->n != B->m) return (NULL) ;
    m = A->m ; anz = A->p [A->n] ;
    n = B->n ; Bp = B->p ; Bi = B->i ; Bx = B->x ; bnz = Bp [n] ;
    w = cs_calloc (m, sizeof (int)) ;                    /* get workspace */
    values = (A->x != NULL) && (Bx != NULL) ;
    x = values ? cs_malloc (m, sizeof (double)) : NULL ; /* get workspace */
    C = cs_spalloc (m, n, anz + bnz, values, 0) ;        /* allocate result */
    if (!C || !w || (values && !x)) return (cs_done (C, w, x, 0)) ;
    Cp = C->p ;
    for (j = 0 ; j < n ; j++)
    {
        if (nz + m > C->nzmax && !cs_sprealloc (C, 2*(C->nzmax)+m))
        {
            return (cs_done (C, w, x, 0)) ;             /* out of memory */
        } 
        Ci = C->i ; Cx = C->x ;         /* C->i and C->x may be reallocated */
        Cp [j] = nz ;                   /* column j of C starts here */
        for (p = Bp [j] ; p < Bp [j+1] ; p++)
        {
            nz = cs_scatter_sgn(A, Bi [p], j, Bx ? Bx [p] : 1, w, x, j+1, C, nz) ;
        }
        if (values) for (p = Cp [j] ; p < nz ; p++) Cx [p] = x [Ci [p]] ;
    }
    Cp [n] = nz ;                       /* finalize the last column of C */
    cs_sprealloc (C, 0) ;               /* remove extra space from C */
    return (cs_done (C, w, x, 1)) ;     /* success; free workspace, return C */
}

/*
This is also very close to Tim Davis's cs_scatter function. This version has
been modified for use in VarInt. Specifically, 
*/
int cs_scatter_sgn(const cs *A,int j,int n,double beta,int *w,double *x,int mark,cs *C,int nz){
	int i, p, *Ap, *Ai, *Ci ;
    double *Ax ;
    if (!CS_CSC (A) || !w || !CS_CSC (C)) return (-1) ;     /* check inputs */
    Ap = A->p ; Ai = A->i ; Ax = A->x ; Ci = C->i ;
    for (p = Ap [j] ; p < Ap [j+1] ; p++)
    {
        i = Ai [p] ;                            /* A(i,j) is nonzero */
        if (w [i] < mark)
        {
            w [i] = mark ;                      /* i is new entry in column j */
            Ci [nz++] = i ;                     /* add i to pattern of C(:,j) */
            if (x) x [i] = ((i==n)?abs(beta*Ax [p]):-abs(beta*Ax [p]));      /* x(i) = beta*A(i,j) */
        }
        else if (x) x [i] += ((i==n)?abs(beta*Ax [p]):-abs(beta*Ax [p]));    /* i exists in C(:,j) already */
    }
    return (nz) ;
}

/*
This is a direct copy of Tim Davis's cs_gaxpy function meant for manipulating
float arrays.
*/
int cs_gaxpy_float(const cs* A,const float* x,float *y){
	
	int p, j, n, *Ap, *Ai ;
    double *Ax ;
    if (!CS_CSC (A) || !x || !y) return (0) ;       /* check inputs */
    n = A->n ; Ap = A->p ; Ai = A->i ; Ax = A->x;
    for (j = 0 ; j < n ; j++)
    {
        for (p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            y [Ai [p]] += (float)(Ax [p] * x [j]);
        }
    }
    return (1) ;
}

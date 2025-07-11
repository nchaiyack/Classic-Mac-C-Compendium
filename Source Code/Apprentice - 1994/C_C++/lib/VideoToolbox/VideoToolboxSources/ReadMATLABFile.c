/*
ReadMATLABFile.c - load/save a matrix from/to a MATLAB data file.
These functions allow any C program to read and write MATLAB data files.

int SaveMatDoubles(FILE *f,char *name,long rows,long cols
	,double *real,double *imag);			// write doubles as 8-byte DOUBLES
int SaveMatShorts(FILE *f,char *name,long rows,long cols
	,short *real,short *imag);				// write shorts as shorts
int LoadNextMatIntoDoubles(FILE *f,char *name,long *rows,long *cols
	,double **real,double **imag);			// read 8-byte DOUBLES into doubles
int LoadNextMatIntoShorts(FILE *f,char *name,long *rows,long *cols
	,short **real,short **imag);			// read 8-byte DOUBLES into shorts
int LoadNamedMatIntoDoubles(FILE *f,char *name,long *rows,long *cols
	,double **real,double **imag);			// read 8-byte DOUBLES into doubles
int LoadNamedMatIntoShorts(FILE *f,char *name,long *rows,long *cols
	,short **real,short **imag);			// read 8-byte DOUBLES into shorts

All the routines assume an open file, and return an error if they hit end of
file. The SaveXXX routines append a matrix to the existing file. The LoadNextXXX
routines read the next matrix from the file, advancing the file pointer.
SaveXXX and LoadNextXXX can be called repeatedly until they fail (e.g. end of
file is reached). The LoadNamedXXX routines first rewind the file and then scan
it until they find a matrix whose name matches that supplied, or end of file is
reached. The file pointer is left at the end of that matrix. Calling LoadNameXXX
repeatedly will read the same matrix every time: the first instance of the named
matrix.

The LoadXXX and SaveXXX routines return 0 if successful and a positive nonzero
integer if they fail.  The two array pointers will be NULL after calling LoadXXX
unless they are loaded. The second (imaginary) pointer will be NULL unless the
matrix was complex. Similarly, when you call SaveXXX you indicate a real matrix
by supplying a NULL second pointer.

There is a slight asymmetry here that you should be aware of. SaveMatShorts()
produces a (compact) MATLAB file made up of shorts, but the
LoadXXXMatIntoShorts() will only read a (big) MATLAB file made up of 8-byte
doubles. So you won't always be able to read from C the file you wrote from C.
The reason for this is that while the MATLAB "Load" command supports all the
MATLAB file formats, the MATLAB "Save" command has no provision for specifying
(binary) file formats and thus always uses format type 0, 8-byte short doubles,
so I didn't bother to implement any routines to read any other format. (It would
be easy to add this capability, if it were needed.)

EXAMPLES:
This example loads the first two matrices from a MATLAB file:

	FILE *f;
	char name[64];
	long rows,cols;
	double *xr,*xi;
	short *mr,*mi;
	int error;
	f = fopen("foo.mat","rb");
	error=LoadNextMatIntoDoubles(f,name,&rows,&cols,&xr,&xi);
 	error=LoadNextMatIntoShorts(f,name,&rows,&cols,&mr,&mi);
	fclose(f);
	free(xr);
	free(xi);
	free(mr);
	free(mi);

Alternatively, you can read a specific matrix, by name, from the file:

	error=LoadNamedMatIntoDoubles(f,"Put name here",&rows,&cols,&xr,&xi);
 	error=LoadNamedMatIntoShorts(f,"Put name here",&rows,&cols,&mr,&mi);

This example creates a MATLAB file containing four matrices:

	FILE *fp;
	double xyz[1000],ar[1000],ai[1000];
	short m[100],n[100];
	fp = fopen("bar.mat","wb");
	SetFileInfo("bar.mat",'MATW','MATL');
	error=SaveMatDoubles(fp,"xyz",2,3,xyz,NULL);	// real
	error=SaveMatShorts(fp,"m",2,3,m,NULL);			// real
	error=SaveMatDoubles(fp,"a",5,5,ar,ai);			// complex
	error=SaveMatShorts(fp,"mc",2,3,m,n);			// complex
	fclose(fp);


HISTORY:
11-3-86 J.N. Little wrote loadmat.c and savemat.c

[14-Feb-91] jmb -- Added support for MPW C 3.x and THINK C.

1/4/93	dgp Renamed to MatLab.c. DOUBLE is now used soley within the new
WriteXXX and ReadXXX subroutines. C users now only deal with double and short
arrays; necessary conversions are done behind the scenes. double is faster and
more convenient for Macintosh programming than the 8-byte short double used in
the MATLAB files. Added support for file format 3: signed short ints. Omitted
the imagf flag since it's redundant. Omitted the type argument by supplying
several user-callable front ends, each tailored to a particular number type.

2/93 dgp Renamed to ReadMatLabFile.c

6/22-25/93 jas & dgp wrote LoadNamedMat. Renamed LoadMatShorts and
LoadMatDoubles to LoadNextMatIntoShorts and LoadNextMatIntoDoubles. Added
LoadNamedMatIntoShorts and LoadNamedMatIntoDoubles.

12/93 dgp Renamed to ReadMATLABFile.c
*/
#include "VideoToolbox.h"
#include <assert.h>

double *ReadDoubles(FILE *f,long elements,char *name);
short *ReadDoublesIntoShorts(FILE *f,long elements,char *name);
int WriteDoubles(FILE *f,long elements,char *name,double *d);
int WriteShorts(FILE *f,long elements,char *name,short *d);
int SaveMat(FILE *f,char *name,long rows,long cols,void *real,void *imag,long type);
int LoadNamedMat(FILE *f,const char *name,long *rows,long *cols
	,void **real,void **imag,long *fileType,long desiredNumberType);
int LoadNextMat(FILE *f,char *name,long *rows,long *cols
	,void **real,void **imag,long *fileType,long desiredNumberType);

typedef struct {
     long type;		/* type */
     long rows;		/* row dimension */
     long cols;		/* column dimension */
     long imagf;	/* flag indicating imag part */
     long namlen;	/* name length (including NULL) */
} Fmatrix;

/* From cmex.h */
// MATLAB files with number format 0 use 8-byte floating point numbers, which
// we'll call DOUBLE.
#undef DOUBLE
#ifdef THINK_C
	#define DOUBLE short double
#else
	#define DOUBLE double
#endif

int LoadNextMatIntoDoubles(FILE *f,char *name,long *rows,long *cols
	,double **real,double **imag)
{
	long fileType,desiredNumberType=0;
	
	return LoadNextMat(f,name,rows,cols,(void **)real,(void **)imag
		,&fileType,desiredNumberType);
}
	
int LoadNextMatIntoShorts(FILE *f,char *name,long *rows,long *cols
	,short **real,short **imag)
{
	long fileType,desiredNumberType=3;
	
	return LoadNextMat(f,name,rows,cols,(void **)real,(void **)imag
		,&fileType,desiredNumberType);
}

int LoadNamedMatIntoDoubles(FILE *f,const char *name,long *rows,long *cols
	,double **real,double **imag)
{
	long fileType,desiredNumberType=0;
	
	return LoadNamedMat(f,name,rows,cols,(void **)real,(void **)imag
		,&fileType,desiredNumberType);
}
	
int LoadNamedMatIntoShorts(FILE *f,const char *name,long *rows,long *cols
	,short **real,short **imag)
{
	long fileType,desiredNumberType=3;
	
	return LoadNamedMat(f,name,rows,cols,(void **)real,(void **)imag
		,&fileType,desiredNumberType);
}

int SaveMatDoubles(FILE *f,char *name,long rows,long cols,double *real,double *imag)
{
	return SaveMat(f,name,rows,cols,real,imag,0);
}

int SaveMatShorts(FILE *f,char *name,long rows,long cols,short *real,short *imag)
{
	return SaveMat(f,name,rows,cols,real,imag,30);
}
	
int LoadNamedMat(FILE *f,const char *name,long *rows,long *cols
	,void **real,void **imag,long *fileType,long desiredNumberType)
{
	char curname[100];
	int error;
	
	rewind(f);
	while(1){
		error=LoadNextMat(f,curname,rows,cols,real,imag,fileType,desiredNumberType);
		if(error)return error;
		if(strcmp(curname,name)==0)break;
		if(*real!=NULL){
			free(*real);
			*real=NULL;
		}
		if(*imag!=NULL){
			free(*imag);
			*imag=NULL;
		}
	}
	return 0;
}

int LoadNextMat(FILE *f,char *name,long *rows,long *cols
	,void **real,void **imag,long *fileType,long desiredNumberType)
{
	Fmatrix	x;
	long elements,namlen,imagf,numberFormat;
	
	*real=*imag=NULL;
	name[0]=0;
	if(feof(f))return 1;

	// Get Fmatrix structure from file
	if (fread((char *)&x,sizeof(Fmatrix),1,f) != 1) return 2;
	*fileType = x.type;
	*rows = x.rows;
	*cols = x.cols;
	namlen = x.namlen;
	elements = x.rows * x.cols;

	if(x.type/1000!=1)return 3;		// not Macintosh-compatible binary format
	if(x.type/100%10!=0)return 4;	// transposed
	numberFormat=x.type/10%10;
	if(numberFormat!=0)return 5;	// not 8-byte doubles

	// Get matrix name from file
	if (fread(name,sizeof(char),namlen,f) != namlen) return 6;
	
	// Get Real part of matrix from file
	switch(desiredNumberType){
	case 0:
		*real=ReadDoubles(f,elements,name);
		break;
	case 3:
		*real=ReadDoublesIntoShorts(f,elements,name);
		break;
	default:
		*real=NULL;
	}
	if(*real==NULL)return 7;

	// Get Imag part of matrix from file, if it exists
	if (x.imagf) {
		switch(desiredNumberType){
		case 0:
			*imag=ReadDoubles(f,elements,name);
			break;
		case 3:
			*imag=ReadDoublesIntoShorts(f,elements,name);
			break;
		}
		if(*imag==NULL){
			free(*real);
			*real=NULL;
			return 8;
		}
	}
	return 0;
}

int SaveMat(FILE *f,char *name,long rows,long cols,void *real,void *imag,long type)
{
	Fmatrix	x;
	long elements;
	int error;
	int fileFormat;
	
	type%=100;	// not transposed
	type+=1000;	// indicate Macintosh-compatible binary format
	x.type = type;
	x.rows = rows;
	x.cols = cols;
	if(imag==NULL)x.imagf=0;
	else x.imagf=1;
	x.namlen = strlen(name) + 1;
	elements = x.rows * x.cols;

	if(x.type/1000!=1)return 9;		// not Macintosh-compatible binary format
	if(x.type/100%10!=0)return 10;	// transposed
	fileFormat=x.type/10%10;
	if(fileFormat!=0 && fileFormat!=3)return 11; // neither 8-byte double, nor short

	fwrite(&x,sizeof(Fmatrix),1,f);
	fwrite(name,sizeof(char),(long)x.namlen,f);
	switch(fileFormat){
	case 0:
		error=WriteDoubles(f,elements,name,real);
		break;
	case 3:
		error=WriteShorts(f,elements,name,real);
		break;
	default:
		error=1;
	}
	if(error)return 12;
	if (imag!=NULL) {
		switch(fileFormat){
		case 0:
			error=WriteDoubles(f,elements,name,imag);
			break;
		case 3:
			error=WriteShorts(f,elements,name,imag);
			break;
		default:
			error=1;
		}
		if(error)return 13;
	}
	return 0;
}

// Reads DOUBLES (8-byte short doubles) into doubles
double *ReadDoubles(FILE *f,long elements,char *name)
{
	DOUBLE *D;
	double *d;
	long i,n,j;
	const dSize=2048/sizeof(DOUBLE);	// Can be whatever you want. Bigger is faster.
	
	assert(sizeof(DOUBLE)==8);
	D=(DOUBLE *)malloc(dSize*sizeof(DOUBLE));
	if(D==NULL){
		printf("\nError: not enough room for buffer\n");
		return NULL;
	}
	if (!(d = (double *)malloc(elements*sizeof(*d)))) {
		printf("\nError: Variable %s too big to load\n",name);
		return NULL;
	}
	for(i=0;i<elements;){
		if(dSize<elements-i)n=dSize;
		else n=elements-i;
		if(fread(D,sizeof(DOUBLE),n,f) != n) {
			free(d);
			free(D);
			return NULL;
		}
		for(j=0;j<n;j++)d[i++]=D[j];	// convert DOUBLE to double
	}
	free(D);
	return d;
}

// Reads DOUBLES (8-byte short doubles) into shorts
short *ReadDoublesIntoShorts(FILE *f,long elements,char *name)
{
	DOUBLE *D;
	short *d;
	long i,n,j;
	const dSize=2048/sizeof(DOUBLE);	// Can be whatever you want. Bigger is faster.
	
	assert(sizeof(DOUBLE)==8);
	D=(DOUBLE *)malloc(dSize*sizeof(DOUBLE));
	if(D==NULL){
		printf("\nError: not enough room for buffer\n");
		return NULL;
	}
	if (!(d = (short *)malloc(elements*sizeof(*d)))) {
		printf("\nError: Variable %s too big to load\n",name);
		return NULL;
	}
	for(i=0;i<elements;){
		if(dSize<elements-i)n=dSize;
		else n=elements-i;
		if(fread(D,sizeof(DOUBLE),n,f) != n) {
			free(d);
			free(D);
			return NULL;
		}
		for(j=0;j<n;j++)d[i++]=D[j];	// convert DOUBLE to short
	}
	free(D);
	return d;
}

// Writes doubles as DOUBLES (8-byte short doubles)
int WriteDoubles(FILE *f,long elements,char *name,double *d)
{
	DOUBLE *D;
	long i,n,j;
	const dSize=2048/sizeof(DOUBLE);	// Can be whatever you want. Bigger is faster.
	
	assert(sizeof(DOUBLE)==8);
	if (d==NULL) return 14;
	D=(DOUBLE *)malloc(dSize*sizeof(DOUBLE));
	if(D==NULL){
		printf("\nError: not enough room for buffer\n");
		return 15;
	}
	for(i=0;i<elements;){
		if(dSize<elements-i)n=dSize;
		else n=elements-i;
		for(j=0;j<n;j++)D[j]=d[i++];	// convert double to DOUBLE
		if(fwrite(D,sizeof(DOUBLE),n,f) != n) {
			free(D);
			return 16;
		}
	}
	free(D);
	return 0;
}

// Writes shorts as shorts
int WriteShorts(FILE *f,long elements,char *name,short *d)
{
	assert(sizeof(short)==2);
	if (d==NULL) return 17;
	if(fwrite(d,sizeof(short),elements,f) != elements) return 18;
	return 0;
}

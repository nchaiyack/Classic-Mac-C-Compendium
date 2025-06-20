
/*
*  Declarations for externally callable routines for the DataScope server
*  on UNIX machines.
*
*  Add your external function to each of the lists.
*  The internal routine, when compiled, will automatically register the
*  routine name and which function to call.
*  
*  The routine name does not need to match the function name string.
*/

int 
	NORM(),					/* FORTRAN declaration */
	exhifilter(),				/* C declaration */
	scos(),
	smooth(),					/* add your entry here */
	puts();						/* dummy entry, anchors list */


struct flist {
	char *namestring;
	int (*fncall)();

};


struct flist dsc[] = {
	"hifilter",exhifilter,
	"sincos",scos,
	"smooth",smooth,			/* add your entry here */
	"",puts

	};


struct flist dsf[] = {			/* FORTRAN calls list */
	"norm",NORM,
	"",puts

	};




/*
*  Declarations for externally callable routines for the DataScope server
*  on SUN UNIX machines.
*
*  Add your external function to each of the lists.
*  The internal routine, when compiled, will automatically register the
*  routine name and which function to call.
*  
*  The routine name does not need to match the function name string.
*  
*  There is one important qualification:  fortran subroutine references
*  called from "C" must have an underscore (_) appended to the
*  reference in the "C" code.  In the example shown below, a FORTRAN
*  subroutine norm() is referenced as norm_(); in the actual FORTRAN
*  source code, this underscore is NOT included.
*
*  Remember that on SUN4 systems, you cannot run external subroutines
*  unless your IP number is hardcoded into the system table as a remote
*  host; this is because DataScope uses the rexecd socket, where this
*  restriction applies.
*/

int 
	norm_(),					/* FORTRAN declaration */
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
	"norm",norm_,
	"",puts

	};



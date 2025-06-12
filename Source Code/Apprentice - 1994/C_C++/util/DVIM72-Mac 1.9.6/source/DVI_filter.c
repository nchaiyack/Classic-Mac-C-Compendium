/* ---------------------------- Dvi_filter --------------------------- */
#include <ctype.h>

pascal Boolean Dvi_filter( ParmBlkPtr param_block );



pascal Boolean Dvi_filter( ParmBlkPtr param_block )
{
	static char suffix[5] = ".dvi";
	register char *fname_ptr;
	register short fname_len, i;
	
	if ( (*param_block).fileParam.ioFlFndrInfo.fdType == 'ODVI' )
		return( FALSE ); /* Display files of type ODVI */
	fname_ptr = (char *)(*param_block).fileParam.ioNamePtr;
	fname_len = *fname_ptr;
	if (fname_len < 4)
		return( TRUE ); /* can't end in ".dvi" with fewer than 4 bytes */
	
	for (i = 3; i >= 0; --i)
	{
		if (tolower(fname_ptr[fname_len]) != suffix[i])
			break;
		--fname_len;
	}
	
	return( i >= 0 );
}


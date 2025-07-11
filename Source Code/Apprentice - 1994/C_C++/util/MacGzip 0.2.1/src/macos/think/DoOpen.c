/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include "thePrefs.h"
#include "MacErrors.h"

#include <string.h>
#include <stdio.h>

#define EMPTY_STR "\0           "

long GvRefNum;

extern int gzip_main (int argc, char **argv);

OSErr DoOpen( Str255 fName, long vRefNum)
{
	int argc;
	char *argv[5]={"gzip",EMPTY_STR,NULL,NULL,NULL};

	char sTemp[256];
	int result;
	
	ParamBlockRec myParamBlock;

/*********** ResFork ***************/

	myParamBlock.fileParam.ioCompletion=NULL;
	myParamBlock.fileParam.ioNamePtr=fName;
	myParamBlock.fileParam.ioVRefNum=vRefNum;
	myParamBlock.fileParam.ioFRefNum=0;
	myParamBlock.fileParam.ioFDirIndex=0 ;
	
	sprintf(sTemp,"Discard Resource Fork: %#s?",fName);
	
	PBGetFInfo( &myParamBlock, false );
	if ( myParamBlock.fileParam.ioFlRLgLen != 0 )
	{
		if ( Cask( sTemp ) == cancel )
			return(128);
	}


/*********** args ***************/
	argc=1;
	
/*********** 1st arg ***************/

	strcpy(argv[argc],"-N");

	if (!currPrefs.compress)							/* gunzip */
	{
		strcat(argv[argc],currPrefs.ascii?"da":"d");	/* ascii? */
	}
	else												/* gzip */
	{
		sprintf(argv[argc],"-%d",currPrefs.level);		/* # */
	}

	if(currPrefs.force)
		strcat(argv[argc],"f");							/* force */
	
	argc++;
	
/*********** 2nd arg ?**************/

	if ( *currPrefs.suffix != 0x00 )
	{
		sprintf((char*)sTemp,"-S%#s",(char*)currPrefs.suffix);
		argv[argc]=(char*)sTemp;
		argc++;
	}

/*********** next arg ***************/

	PtoCstr(fName);
	argv[argc]=(char*)fName;
	argv[++argc]=NULL;
	
/*********** do it ***************/

	SetVol( nil, vRefNum);
	GvRefNum=vRefNum;
	
	result = gzip_main ( argc, argv );
	
/*********** creator & other ***************/

	CtoPstr(fName);

	return( result );

}


/**********************************************************************
 *	This file contains the functions which initialize the core 
 *	AppleEvents for the program.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.AE.h"



/**********************************************************************
 *	Function Open_AE(), this function supports what happens when a file
 *	is dropped on Creator Changer.  I only support ONE file at a time
 *	for this version of Creator Changer.
 **********************************************************************/

pascal OSErr Open_AE(AppleEvent *the_event, AppleEvent *the_reply, long ref_con)
	{
	
	AEDescList	the_files;
	AEKeyword	key_word;
	DescType	the_type;
	Size		the_size;
	
	AEGetParamDesc(the_event, keyDirectObject, typeAEList, &the_files);
	AECountItems(&the_files, &Num_Of_Files);
	
	if(Num_Of_Files==1)
		{
		Multiple_Files=NO;
		AEGetNthPtr(&the_files, 1, typeFSS, &key_word, &the_type, &The_File_Spec, sizeof(The_File_Spec), &the_size);
		FSpGetFInfo(&The_File_Spec, &File_Info);
		AEDisposeDesc(&the_files);
		}
	else Multiple_Files=YES;
	
	Open_Changer_DLOG(&the_files);
	
	Multiple_Files=NO;
	Num_Of_Files=1;
	
	return(noErr);
	
	}



/**********************************************************************
 *	Function Quit_AE(), this function supports the 'quit' event.  All
 *	that it does is set the global switch to true to quit.
 **********************************************************************/

pascal OSErr Quit_AE(AppleEvent *theEvent, AppleEvent *reply, long ref_con)
	{
	
		//	Set the global on / off switch to true, GOODBYE!
	All_Done=TRUE;
	return(noErr);
	
	}



/**********************************************************************
 *	Function Print_AE(), this function does nothing, besides report 
 *	that CC does not support the print event.
 **********************************************************************/

pascal OSErr Print_AE(AppleEvent *theEvent, AppleEvent *reply, long ref_con)
	{
	
	return(errAEEventNotHandled);
	
	}



/**********************************************************************
 *	Function Start_AE(), this function controls what happens when 
 * Creator Changer is launched.
 **********************************************************************/

pascal OSErr Start_AE(AppleEvent *theEvent, AppleEvent *reply, long ref_con)
	{
	
	Handle_Options_Choice(O_OPEN_ITEM);
	
	return(noErr);
	
	}

/*****
 * CShellApp.c
 *
 *	Application methods for a typical application.
 *
 *  Copyright � 1990 Symantec Corporation.  All rights reserved.
 *
 *****/

#include "CShellApp.h"
#include "CShellDialog.h"
#include "CDLOGDirector.h"

	// to suppress class stripping only
#include "CButton.h"
#include "CRadioControl.h"
#include "CRadioGroupPane.h"
#include "CStdPopupPane.h"
#include "CCheckBox.h"
#include "CIconPane.h"
#include "CIntegerText.h"
#include "CMovieController.h"

extern	OSType					gSignature;
extern 	CBartender				*gBartender;		/* Manages all menus				*/

#define		cmdNewControllerFromRes		1025
#define		cmdNewControllerFromFile	1026

#define		kExtraMasters		4
#define		kRainyDayFund		20480
#define		kCriticalBalance	20480
#define		kToolboxBalance		20480

#define		kShellDLOGID		1024

/***
 * IShellApp
 *
 *	Initialize the application. Your initialization method should
 *	at least call the inherited method. If your application class
 *	defines its own instance variables or global variables, this
 *	is a good place to initialize them.
 *
 ***/

void CShellApp::IShellApp(void)

{
	CApplication::IApplication( kExtraMasters, kRainyDayFund, 
						kCriticalBalance, kToolboxBalance);
    

/*  The parameters to IApplication are the number of times to call  
    MoreMasters, the total number of bytes of heap space to reserve for                       
    monitoring low memory situations, and the portion of the memory
    reserve to set aside for critical operations and toolbox calls.
    
    Four (4) is a reasonable number of MoreMasters calls,                           
    but you should determine a good number for your application                     
    by observing the heap using Lightsbug,                                              
    TMON, or Macsbug. Set this parameter to zero, give your                         
    program a rigorous work-out, then look at the heap and count                        
    how many master pointer blocks have been allocated. Master                      
    pointer blocks are nonrelocatable and have a size of $100                           
    (hex). You should call MoreMasters at least this many                               
    times -- add a few extra just to be safe. The purpose of all                        
    this preflighting is to prevent heap fragmentation. You                             
    don't want the Memory Manager to call MoreMasters and                           
    create a nonrelocatable block in the middle of your heap. By                        
    calling MoreMasters at the very beginning of the program,                           
    you ensure that these blocks are allocated in a group at the                        
    bottom of the heap. 
                                                                        
    The memory reserve is a safeguard for handling low memory                   
    conditions and is used by the GrowMemory method in                              
    CApplication (check there for more comments). In general,                           
    your program should never request a memory block greater                        
    than this reserve size without explicitly checking in                               
    advance whether there is enough free memory to satisfy the                      
    the request.
                                                                                    
 */

}



/***
 * SetUpFileParameters
 *
 *	In this routine, you specify the kinds of files your
 *	application opens.
 *
 *
 ***/

void CShellApp::SetUpFileParameters(void)

{
	inherited::SetUpFileParameters();	/* Be sure to call the default method */

		/**
		 **	sfNumTypes is the number of file types
		 **	your application knows about.
		 **	sfFileTypes[] is an array of file types.
		 **	You can define up to 4 file types in
		 **	sfFileTypes[].
		 **
		 **/

	sfNumTypes = 1;
	sfFileTypes[0] = 'TEXT';

		/**
		 **	Although it's not an instance variable,
		 **	this method is a good place to set the
		 **	gSignature global variable. Set this global
		 **	to your application's signature. You'll use it
		 **	to create a file (see CFile::CreateNew()).
		 **
		 **/

	gSignature = '?\??\?';
}

/***
 * SetUpMenus 
 *
 * Set up menus which must be created at run time, such as a
 * Font menu. You can eliminate this method if your application
 * does not have any such menus.
 *
***/

 void CShellApp::SetUpMenus()
 {

  inherited::SetUpMenus();  /*  Superclass takes care of adding     
                                menus specified in a MBAR id = 1    
                                resource    
                            */                          

        /* Add your code for creating run-time menus here */    
 }



/***
 * DoCommand
 *
 *	Your application will probably handle its own commands.
 *	Remember, the command numbers from 1-1023 are reserved.
 *  The file Commands.h contains all the predefined TCL
 *  commands.
 *
 *	Be sure to call the default method, so you can get
 *	the default behvior for standard commands.
 *
 ***/
void CShellApp::DoCommand(long theCommand)

{
	CDirector 	*director;

	switch (theCommand) {
	
		/* Your commands go here */
		case cmdNewControllerFromRes:
			director = DoShellDialogClass(FALSE, 1025);
			break;
			
		case cmdNewControllerFromFile:
			director = DoShellDialogClass(FALSE, 1024);
			break;
			
		case cmdNew:
			break;	
		default:	inherited::DoCommand(theCommand);
					break;
	}
}


/***
 *
 * UpdateMenus 
 *
 *   Perform menu management tasks
 *
***/

 void CShellApp::UpdateMenus()
 {
  inherited::UpdateMenus();     /* Enable standard commands */      

    /* Enable the commands handled by your Application class */ 
    gBartender->EnableCmd(cmdNewControllerFromRes);
    gBartender->EnableCmd(cmdNewControllerFromFile);
 }


/***
 * Exit
 *
 *	Chances are you won't need this method.
 *	This is the last chance your application gets to clean up
 *  things like temporary files before terminating.
 *
 ***/
 
void CShellApp::Exit()

{
	/* your exit handler here */
}


/******************************************************************************
 ForceClassReferences
 
 	This method creates dummy references to classes that we don't want
 	stripped out by the linker when the application is built. This could
 	happen if the class is only created via new_by_name and is never directly
 	referenced. CApplication automatically calls this method.
 	
******************************************************************************/


void	CShellApp::ForceClassReferences( void)
{
	Boolean alwaysFalse = FALSE;
	CObject *dummy = NULL;
	
	if (alwaysFalse == TRUE)
	{
		member( dummy, CButton);
		member( dummy, CCheckBox);
		member( dummy, CRadioControl);
		member( dummy, CRadioGroupPane);
		member( dummy, CIconPane);
		member( dummy, CIntegerText);
		member( dummy, CStdPopupPane);
		member( dummy, CMovieController);
	}
}


/******************************************************************************
 DoShellDialogClass
 
 	Like DoShellDialog but creates the object from a specific CShellDialog lass.
******************************************************************************/

CDirector *CShellApp::DoShellDialogClass(Boolean modal, short ShellDialogID)
{
	CShellDialog		*dialog = NULL;
	long			cmd;
	
	TRY
	{
		dialog = new CShellDialog;	
		dialog->IShellDialog(ShellDialogID);
		dialog->BeginDialog();

		if (modal)
		{
			cmd = dialog->DoModalDialog( cmdOK);	
			ForgetObject( dialog);
		}
	}
	CATCH
	{
		ForgetObject( dialog);
	}
	ENDTRY;
	
	return dialog; // not null if dialog is non-modal

}



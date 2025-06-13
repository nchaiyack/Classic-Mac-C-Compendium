//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsDirector.cp
//|
//| This implements the controls dialog director
//|_________________________________________________________

#include "CEditMouseControlsDirector.h"
#include "CHyperCuberPrefs.h"
#include "CMouseControlsArrayPane.h"
#include "CMouseControlsDialog.h"
#include "CMouseControlsDirector.h"

#include "HyperCuber Commands.h"
#include "HyperCuber Errors.h"
#include "Mouse.h"

#include <CPopupMenu.h>
#include <CStdPopupPane.h>

//============================ Globals ============================\\

extern CDesktop 		*gDesktop;
extern CHyperCuberPrefs *gPrefs;


//============================ Prototypes ============================\\

extern void GeneralError(short error);



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CMouseControlsDirector::IMouseControlsDirector
//
// Purpose: Initialize the controls dialog.
//
// Parameters: none
//_________________________________________________________

void CMouseControlsDirector::IMouseControlsDirector (CDirectorOwner *aSupervisor)
{

#define CONTROLS_DIALOG_ID	131

	CDialogDirector::IDialogDirector (aSupervisor);			//  Init superclass

	array = new(CArray);									//  Set up the array
	array->IArray(sizeof(MouseControl));

	array_pane = new (CMouseControlsArrayPane);				//  Create the array pane

	BuildArrayFromPrefs();									//  Build the array

	CMouseControlsDialog *dialog = new (CMouseControlsDialog);	//  Set up the mouse controls dialog
	dialog->IMouseControlsDialog(CONTROLS_DIALOG_ID, gDesktop,
									this, array, array_pane);
	itsWindow = dialog;

}	//=== CMouseControlsDirector::IMouseControlsDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsDirector::BuildArrayFromPrefs
//|
//| Purpose: These procedure builds the array from the preferences.
//|
//| Parameters: none
//|______________________________________________________________________________

void	CMouseControlsDirector::BuildArrayFromPrefs(void)
{

	while(array->GetNumItems())								//  Clear array
		array->DeleteItem(1);

	long i;
	for (i = 0; i < gPrefs->prefs.num_mouse_controls; i++)
		array->InsertAtIndex(
				&(gPrefs->prefs.mouse_controls[i]), 10000);	//  Add this mouse control to array

}	//==== CMouseControlsDirector::BuildArrayFromPrefs() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsDirector::DoCommand
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CMouseControlsDirector::DoCommand( long aCmd)
{

	Cell					cell;
	MouseControl	mouse_control;
	Boolean					selection;

	switch (aCmd)
	{
		
		case cmdDefaults:
		
			PrefsStruct prefs = gPrefs->prefs;		//  Save the current prefs
			gPrefs->SetDefaults();					//  Set all preferences to default values

			BuildArrayFromPrefs();					//  Rebuild the array with default values
			
			gPrefs->prefs = prefs;					//  Restore the current prefs
			break;
	
		case cmdEdit:
		
			cell.h = cell.v = 0;
			selection = array_pane->GetSelect(TRUE, &cell);		//  Find highlighted cell, if any
			if (!selection) break;								//  Don't do anything if no selection
			array->GetArrayItem(&mouse_control, cell.v + 1);	//  Get the cell
		
			CEditMouseControlsDirector ekcdialog_director;
			ekcdialog_director.IEditMouseControlsDirector(this, &mouse_control);
			ekcdialog_director.TalkToUser();

			array->SetArrayItem(&mouse_control, cell.v + 1);	//  Change the cell
			
			break;
		
		case cmdAdd:

			if (array->GetNumItems() == MAX_MOUSE_CONTROLS)
				{
				GeneralError(too_many_mouse_controls);			//  Don't allow number of controls
																//   to exceed limit.
				break;
				}

			cell.h = cell.v = 0;
			selection = array_pane->GetSelect(TRUE, &cell);		//  Find highlighted cell, if any
			
			mouse_control.dimension = 3;						//  Create a new mouse command
			mouse_control.angle = 1;
			mouse_control.horiz = TRUE;
			mouse_control.multiplier = 1;
			mouse_control.modifiers = 0;
			
			array->InsertAtIndex(&mouse_control, (selection) ?
							cell.v + 1 : 10000);				//  Insert a new cell before
																//   selection or at end if none.
			break;

		case cmdRemove:
		
			cell.h = cell.v = 0;
			selection = array_pane->GetSelect(TRUE, &cell);		//  Find highlighted cell, if any
			
			if (selection)
				array->DeleteItem(cell.v + 1);					//  Remove the cell

			break;
		
		
		default:
			inherited::DoCommand( aCmd);
			break;
	}
	
}	//=== CMouseControlsDirector::DoCommand ===\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CMouseControlsDirector::TalkToUser
//
// Purpose: Handle commands for the dialog.
//
// Parameters: none
//_________________________________________________________

void CMouseControlsDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	if (dismiss == cmdOK)
		{

		PrefsStruct *prefs = &(gPrefs->prefs);					//  Get pointer to current prefs
		prefs->num_mouse_controls = array->GetNumItems();

		long i;
		for (i = 0; i < prefs->num_mouse_controls; i++)
			{
			MouseControl mouse_control;
		
			array->GetArrayItem(&mouse_control, i + 1);			//  Get the mouse control
			prefs->mouse_controls[i] = mouse_control;			//  Move this mouse control to prefs
			}
		
		}
	
}	//=== CMouseControlsDirector::TalkToUser ===\\

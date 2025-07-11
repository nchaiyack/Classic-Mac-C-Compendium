//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsDirector.cp
//|
//| This implements the controls dialog director
//|_________________________________________________________

#include "CEditKeyControlsDirector.h"
#include "CHyperCuberPrefs.h"
#include "CKeyControlsArrayPane.h"
#include "CKeyControlsDialog.h"
#include "CKeyControlsDirector.h"

#include "HyperCuber Commands.h"
#include "HyperCuber Errors.h"
#include "Keys.h"

#include <CPopupMenu.h>
#include <CStdPopupPane.h>

//============================ Globals ============================\\

extern CDesktop 		*gDesktop;
extern CHyperCuberPrefs *gPrefs;


//========================== Prototypes ===========================\\

extern void GeneralError(short error);



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CKeyControlsDirector::IKeyControlsDirector
//
// Purpose: Initialize the controls dialog.
//
// Parameters: none
//_________________________________________________________

void CKeyControlsDirector::IKeyControlsDirector (CDirectorOwner *aSupervisor)
{

#define CONTROLS_DIALOG_ID	131

	CDialogDirector::IDialogDirector (aSupervisor);			//  Init superclass

	array = new(CArray);									//  Set up the array
	array->IArray(sizeof(KeyControl));

	array_pane = new (CKeyControlsArrayPane);				//  Create the array pane

	BuildArrayFromPrefs();									//  Build the array

	CKeyControlsDialog *dialog = new (CKeyControlsDialog);	//  Set up the key controls dialog
	dialog->IKeyControlsDialog(CONTROLS_DIALOG_ID, gDesktop,
									this, array, array_pane);
	itsWindow = dialog;

}	//=== CKeyControlsDirector::IKeyControlsDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsDirector::BuildArrayFromPrefs
//|
//| Purpose: These procedure builds the array from the preferences.
//|
//| Parameters: none
//|______________________________________________________________________________

void	CKeyControlsDirector::BuildArrayFromPrefs(void)
{

	while(array->GetNumItems())								//  Clear array
		array->DeleteItem(1);

	long i;
	for (i = 0; i < gPrefs->prefs.num_key_controls; i++)
		array->InsertAtIndex(
				&(gPrefs->prefs.key_controls[i]), 10000);	//  Add this key control to array

}	//==== CMouseControlsDirector::BuildArrayFromPrefs() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsDirector::DoCommand
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CKeyControlsDirector::DoCommand( long aCmd)
{

	Cell				cell;
	KeyControl	key_control;
	Boolean				selection;

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
			array->GetArrayItem(&key_control, cell.v + 1);		//  Get the cell
		
			CEditKeyControlsDirector ekcdialog_director;
			ekcdialog_director.IEditKeyControlsDirector(this, &key_control);
			ekcdialog_director.TalkToUser();

			array->SetArrayItem(&key_control, cell.v + 1);		//  Change the cell
			
			break;
		
		case cmdAdd:

			if (array->GetNumItems() == MAX_KEY_CONTROLS)
				{
				GeneralError(too_many_key_controls);			//  Don't allow number of controls
																//   to exceed limit.
				break;
				}

			cell.h = cell.v = 0;
			selection = array_pane->GetSelect(TRUE, &cell);		//  Find highlighted cell, if any
			
			key_control.dimension = 3;									//  Create a new key control
			key_control.angle = 1;
			key_control.increment = 1;
			key_control.key_code = 0x31;
			key_control.modifiers = 0;
			
			array->InsertAtIndex(&key_control, (selection) ?
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
	
}	//=== CKeyControlsDirector::DoCommand ===\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CKeyControlsDirector::TalkToUser
//
// Purpose: Handle commands for the dialog.
//
// Parameters: none
//_________________________________________________________

void CKeyControlsDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	if (dismiss == cmdOK)
		{

		PrefsStruct *prefs = &(gPrefs->prefs);					//  Get pointer to the current prefs
		prefs->num_key_controls = array->GetNumItems();

		long i;
		for (i = 0; i < prefs->num_key_controls; i++)
			{
			KeyControl key_control;
		
			array->GetArrayItem(&key_control, i + 1);				//  Get the key control
			prefs->key_controls[i] = key_control;				//  Move this key control to prefs
			}
		
		}
	
}	//=== CKeyControlsDirector::TalkToUser ===\\

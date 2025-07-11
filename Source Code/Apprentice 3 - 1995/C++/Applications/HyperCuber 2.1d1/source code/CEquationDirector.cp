//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDirector.cp
//|
//| This implements the equation dialog director
//|_________________________________________________________

#include "CEditFunctionDirector.h"
#include "CEditParameterDirector.h"
#include "CEquationDialog.h"
#include "CEquationDirector.h"
#include "CHyperCuberPrefs.h"
#include "CFunctionsArrayPane.h"
#include "CParametersArrayPane.h"

#include "HyperCuber Commands.h"
#include "HyperCuber Errors.h"
#include "Keys.h"

#include <CPopupMenu.h>
#include <CStdPopupPane.h>

#include <string.h>

//============================ Globals ============================\\

extern CDesktop 		*gDesktop;
extern CHyperCuberPrefs *gPrefs;


//============================ Prototypes ============================\\

extern void GeneralError(short error);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDirector::IEquationDirector
//|
//| Purpose: Initialize the controls dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CEquationDirector::IEquationDirector(CDirectorOwner *aSupervisor)
{

#define EQUATION_DIALOG_ID	135

	CDialogDirector::IDialogDirector (aSupervisor);			//  Init superclass

	parameters_array = new(CArray);							//  Set up the parameters array
	parameters_array->IArray(sizeof(Parameter));
	parameters_array_pane = new (CParametersArrayPane);		//  Create the parameters array pane

	functions_array = new(CArray);							//  Set up the functions array
	functions_array->IArray(MAX_FUNCTION_LENGTH);
	functions_array_pane = new (CFunctionsArrayPane);		//  Create the functions array pane

	BuildArraysFromPrefs();									//  Build the arrays

	CEquationDialog *dialog = new (CEquationDialog);		//  Set up the equation dialog
	dialog->IEquationDialog(EQUATION_DIALOG_ID, gDesktop,
			this, parameters_array, parameters_array_pane,
				functions_array, functions_array_pane);
	itsWindow = dialog;

}	//=== CEquationDirector::IEquationDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDirector::BuildArraysFromPrefs
//|
//| Purpose: These procedure builds the arrays from the preferences.
//|
//| Parameters: none
//|______________________________________________________________________________

void	CEquationDirector::BuildArraysFromPrefs(void)
{

	while(parameters_array->GetNumItems())					//  Clear parameters array
		parameters_array->DeleteItem(1);

	while(functions_array->GetNumItems())					//  Clear functions array
		functions_array->DeleteItem(1);

	long i;
	for (i = 0; i < gPrefs->prefs.num_parameters; i++)
		parameters_array->InsertAtIndex(
				&(gPrefs->prefs.parameters[i]), 10000);		//  Add this parameter to array

	for (i = 0; i < gPrefs->prefs.equation_dimension; i++)
		functions_array->InsertAtIndex(
				&(gPrefs->prefs.functions[i]), 10000);		//  Add this function to array

}	//==== CEquationDirector::BuildArrayFromPrefs() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDirector::DoCommand
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CEquationDirector::DoCommand( long aCmd)
{

	Cell				cell;
	Parameter			parameter;
	char				function[MAX_FUNCTION_LENGTH];
	Boolean				selection;

	switch (aCmd)
	{
		
		case cmdEditParameter:
		
			cell.h = cell.v = 0;
			selection = parameters_array_pane->GetSelect(TRUE, &cell);//  Find highlighted cell, if any
			if (!selection) break;								//  Don't do anything if no selection
			parameters_array->GetArrayItem(&parameter, cell.v + 1);	//  Get the cell
		
			CEditParameterDirector epdialog_director;
			epdialog_director.IEditParameterDirector(this, &parameter);
			epdialog_director.TalkToUser();

			parameters_array->SetArrayItem(&parameter, cell.v + 1);	//  Change the cell
			
			break;
		
		case cmdAddParameter:

			if (parameters_array->GetNumItems() == MAX_PARAMETERS)
				{
				GeneralError(too_many_parameters);						//  Don't allow number of params
																		//   to exceed limit.
				break;
				}

			cell.h = cell.v = 0;
			selection = parameters_array_pane->GetSelect(TRUE, &cell);	//  Find highlighted cell, if any
			
			strcpy(parameter.name, "t");
			parameter.start = -10;
			parameter.end = 10;
			parameter.step = 2;											//  Create a new parameter
			
			parameters_array->InsertAtIndex(&parameter, (selection) ?
							cell.v + 1 : 10000);						//  Insert a new cell before
																		//   selection or at end if none.
			break;

		case cmdRemoveParameter:
		
			cell.h = cell.v = 0;
			selection = parameters_array_pane->GetSelect(TRUE, &cell);	//  Find highlighted cell, if any
			
			if (selection)
				{
				if (parameters_array->GetNumItems() == 1)
					GeneralError(must_have_one_parameter);				//  Don't allow deletion of last
																		//   remaining parameter

				else
					parameters_array->DeleteItem(cell.v + 1);				//  Remove the cell
				}

			break;
		
		case cmdEditFunction:
		
			cell.h = cell.v = 0;
			selection = functions_array_pane->GetSelect(TRUE, &cell);	//  Find highlighted cell, if any
			if (!selection) break;									//  Don't do anything if no selection
			functions_array->GetArrayItem(function, cell.v + 1);			//  Get the cell
		
			CEditFunctionDirector efdialog_director;
			efdialog_director.IEditFunctionDirector(this, cell.v + 1, function);
			efdialog_director.TalkToUser();

			functions_array->SetArrayItem(&function, cell.v + 1);	//  Change the cell
			
			break;
		
		case cmdAddFunction:

			if (functions_array->GetNumItems() == MAX_DIMENSION)
				{
				GeneralError(too_many_dimensions);						//  Don't allow number of functions
																		//   to exceed allowable dimensions.
				break;
				}

			cell.h = cell.v = 0;
			selection = functions_array_pane->GetSelect(TRUE, &cell);	//  Find highlighted cell, if any
			
			strcpy(function, "sin(t)");							//  Create a new function
			
			functions_array->InsertAtIndex(&function, (selection) ?
							cell.v + 1 : 10000);				//  Insert a new cell before
																//   selection or at end if none.
			break;

		case cmdRemoveFunction:
		
			cell.h = cell.v = 0;
			selection = functions_array_pane->GetSelect(TRUE, &cell);	//  Find highlighted cell, if any
			
			if (selection)
				{
				if (functions_array->GetNumItems() == 3)
					GeneralError(must_have_three_functions);			//  Don't allow number of functions
																		//   to drop below three.

				else
					functions_array->DeleteItem(cell.v + 1);			//  Remove the cell
				}

			break;
		
		case cmdAccept:
		case cmdGraph:

			EndDialog(aCmd, TRUE);										//  Exit dialog on Accept or Graph
			break;
		
		default:
		
			inherited::DoCommand( aCmd);
			break;
			
	}
	
}	//=== CEquationDirector::DoCommand ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDirector::TalkToUser
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: graph: receives TRUE if Graph was clicked
//|_________________________________________________________

void CEquationDirector::TalkToUser(Boolean& graph)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdGraph);
	
	if ((dismiss == cmdGraph) || (dismiss == cmdAccept))
		{

		PrefsStruct *prefs = &(gPrefs->prefs);					//  Get pointer to the current prefs
		prefs->num_parameters = parameters_array->GetNumItems();

		long i;
		for (i = 0; i < prefs->num_parameters; i++)
			{
			Parameter parameter;
		
			parameters_array->GetArrayItem(&parameter, i + 1);		//  Get the parameter
			prefs->parameters[i] = parameter;					//  Move this parameter to prefs
			}

		prefs->equation_dimension = functions_array->GetNumItems();

		for (i = 0; i < prefs->equation_dimension; i++)
			functions_array->GetArrayItem(prefs->functions[i], i + 1);//  Move this function to prefs
		
		}
	
	graph = (dismiss == cmdGraph);								//  Only graph if user clicked Graph
	
}	//=== CEquationDirector::TalkToUser ===\\

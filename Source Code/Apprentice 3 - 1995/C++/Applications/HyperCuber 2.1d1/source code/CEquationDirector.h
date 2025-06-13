//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEquationDirector class.  The
//| CEquationDirector class supervises the dialog which is used to define
//| mathematical equations.
//|________________________________________________________________________________

#pragma once

#include <CDialogDirector.h>

class CArray;
class CParametersArrayPane;
class CFunctionsArrayPane;

class CEquationDirector : public CDialogDirector
	{
	
	CArray 					*parameters_array;
	CParametersArrayPane	*parameters_array_pane;			//  The array pane showing parameters

	CArray 					*functions_array;
	CFunctionsArrayPane		*functions_array_pane;			//  The array pane for the functions
	
  public:

	virtual void	IEquationDirector (CDirectorOwner *aSupervisor);
	virtual void	DoCommand(long command);
	virtual void	TalkToUser(Boolean& graph);

	virtual void	BuildArraysFromPrefs(void);

	};

/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/******************************************************************************
 CProjectInfoDialog.c

		A subclass of CDLOGDirector that displays project info dialogs.
		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CProjectInfoDialog.h"
#include "CApplication.h"
#include "Commands.h"
#include "HarvestCommands.h"
#include "CRadioGroupPane.h"
#include "CWindow.h"

/******************************************************************************
 IProjectInfoDialog
 
 	Initialize the dialog. This method also assigns help balloon resource
 	indexes for some of the panes in the window.
 	
******************************************************************************/

void CProjectInfoDialog::IProjectInfoDialog( int theID, CDirectorOwner *aSupervisor)
{
	CDLOGDirector::IDLOGDirector( theID, aSupervisor);
}

void
CProjectInfoDialog::DoCommand(long theCommand)
{
	switch (theCommand) {
		default:
			inherited::DoCommand(theCommand);
	}
}


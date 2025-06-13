/************************************************************************\
 CWASTETask.cpp

	A task for dealing with undo for CWASTEText objects under WASTE 1.1

 by Dan Crevier
 5/19/95
 
 based on code by Jud Spencer

\************************************************************************/

#include "CWASTETask.h"
#include "CWASTEText.h"

#define undoDrag	6

/************************************************************************\
	Constructor - save a pointer to the CWASTEText
\************************************************************************/

CWASTETask::CWASTETask(CWASTEText *text)
	: CTask(0)
{
       wasteText = text;
}

/************************************************************************\
	Destructor - tell text object that we aren't around any more
\************************************************************************/

CWASTETask::~CWASTETask()
{
	if (wasteText->itsLastTask == this)
		wasteText->itsLastTask = NULL;
}
                        
/************************************************************************\
	GetNameIndex - give a pointer to the name of the undo action
\************************************************************************/

short CWASTETask::GetNameIndex()
{
	short offset = -1;
	Boolean undone;
        
	switch (wasteText->GetUndoInfo(&undone))
	{
		case weAKTyping:
			offset = undoTyping;
			break;
			
		case weAKCut:
			offset = undoCut;
			break;
			
		case weAKPaste:
			offset = undoPaste;
			break;
			
		case weAKClear:
			offset = undoClear;
			break;
			
		case weAKDrag:
			offset = undoDrag;
			break;

		case weAKSetStyle:
			offset = undoFormatting;
			break;
	}
	if ( offset >= 0)
	{
		return (offset + CWASTEText::cFirstTaskIndex);
	}
	else
	{
		return ( 0);
	}
}

/************************************************************************\
	Undo - tell the CWASTEText to call WEUndo()
\************************************************************************/

void CWASTETask::Undo()
{
	wasteText->Prepare();
	wasteText->DoUndo();
}

/************************************************************************\
	IsUndone - return true if it's a "redo"
\************************************************************************/

Boolean CWASTETask::IsUndone()
{
	Boolean undone;
	
	wasteText->GetUndoInfo( &undone);

	return undone;
}

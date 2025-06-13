/************************************************************************\
 CWASTETask.h

	A task for dealing with undo for CWASTEText objects under WASTE 1.1

 by Dan Crevier
 5/19/95
 
 based on code by Jud Spencer

\************************************************************************/

#pragma once

#include "CTask.h"
class CWASTEText;

class CWASTETask: public CTask
{
	public:                 
		CWASTETask(CWASTEText *text);
		~CWASTETask();
                                                                
		virtual short			GetNameIndex(void);
		virtual void			Undo(void);
		virtual Boolean			IsUndone(void);
        
	protected:
		CWASTEText				*wasteText;
                
};


/* CDocument.c */

#include "CDocument.h"
#include "CMyApplication.h"


/* */			CDocument::CDocument()
	{
		Application->RegisterDocument(this);
	}


/* */			CDocument::~CDocument()
	{
		Application->DeregisterDocument(this);
	}


/* create a new blank file */
void			CDocument::DoNewFile(void)
	{
		EXECUTE(PRERR(AllowResume,"CDocument::DoNewFile has not ben overridden."));
	}


/* read a file into the data structures */
/* windows will update themselves as necessary */
void			CDocument::DoOpenFile(FSSpec* TheSpec)
	{
		EXECUTE(PRERR(AllowResume,"CDocument::DoOpenFile has not been overridden."));
	}


/* print the document */
void			CDocument::DoPrint(void)
	{
		EXECUTE(PRERR(AllowResume,"CDocument::DoPrint has not been overridden."));
	}


/* make document go away */
/* cause windows to go away.  When all windows are gone, some mechanism */
/* will delete the document's object */
MyBoolean	CDocument::GoAway(void)
	{
		EXECUTE(PRERR(AllowResume,"CDocument::DoGoAway has not been overridden."));
		return False;
	}

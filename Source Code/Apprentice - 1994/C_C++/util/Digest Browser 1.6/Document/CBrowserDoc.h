/****
 * CBrowserDoc.h
 *
 *	Document class for a typical application.
 *
 ****/

#define	_H_CBrowserDoc			/* Include this file only once */
#include <CDocument.h>
#include <CApplication.h> 
#include "CDisplayText.h"
#include "CDisplayIndex.h"
#include "Browser.h"

struct CBrowserDoc : CDocument {

	long index_displayed;
	CDisplayIndex	*itsListIndex;
	CDisplayText	*itsMessage;

				/** Construction **/
	void        IBrowserDoc(CApplication *aSupervisor, Boolean printable);
	void		Dispose();

	void		NewFile(void);		// override to do nothing
	void		OpenFile(SFReply *macSFReply);
	void		BuildWindow(BrowserDirPtr dir);

	void		DoCommand(long theCommand);
	void		UpdateMenus();

	void		SaveTaggedTo(void);
	void		AppendTaggedTo(void);
	void		TagItem(void);
	void		ClearTagged(void);

	OSErr		WriteTagged(short fileRef);
	OSErr		DoCopyResource(ResType theType, short theId,
					short source, short dest);

// JRB addition - new method to handle click in grow box if index window
	void AdjustPaneDivider(Point hitPt, Rect *theDragBox);	/* already in CPane coords */
};
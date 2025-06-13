/****
 * CDisplayIndex.h
 *
 *	A text edit pane for a tiny editor.
 *
 ****/


#define _H_CDisplayIndex

#include "CSelectLine.h"
#include "Browser.h"

struct CDisplayIndex : public CSelectLine {
protected:
	BrowserDirPtr	itsDir;
	BrowserItemPtr	itsSelItem;
public:
								/** Contruction/Destruction **/
	void	IDisplayIndex(CView *anEnclosure, CBureaucrat *aSupervisor,
				short vLoc, short vHeight, BrowserDirPtr theDir, long index_displayed);


	void	SetIndex(long index_displayed);
	void	Dispose(void);

	void	SelectionChanged(void);
	void	SetSelectedLine( long line, Boolean redraw);
	void	TagItem(void);
	void	TagLine(long line, Boolean tag, Boolean setSelection, Boolean redraw);

	BrowserDirPtr	GetDirectory(void);
	BrowserItemPtr	GetSelectedItem(void);

};

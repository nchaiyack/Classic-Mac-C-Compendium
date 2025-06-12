/****
 * CDisplayText.h
 *
 *	A text edit pane for a tiny editor.
 *
 ****/


#define _H_CDisplayText

// JRB change from CEditText to CBrowserEditText
#include "CBrowserEditText.h"

struct CDisplayText : CBrowserEditText {

								/** Contruction/Destruction **/
	void	IDisplayText(CView *anEnclosure, CBureaucrat *aSupervisor,
				short vLoc, short heigth);

	void	DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);

	void	ProviderChanged( CCollaborator *aProvider, long reason,
				void* info);
};

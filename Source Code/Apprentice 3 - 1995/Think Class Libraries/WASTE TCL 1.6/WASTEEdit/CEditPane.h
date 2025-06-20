/****
 * CEditPane.h
 *
 *	A text edit pane for a tiny editor.
 *
 ****/


#define _H_CEditPane

#include "CWASTEText.h"

struct CEditPane : CWASTEText {


								/** Instance Methods **/
									/** Contruction/Destruction **/
	void	IEditPane(CView *anEnclosure, CBureaucrat *aSupervisor);

									/** Command **/
	void	DoCommand(long theCommand);

									/** Mouse and Keystrokes **/
	void	DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);
	void	DoAutoKey(char theChar, Byte keyCode, EventRecord *macEvent);
};

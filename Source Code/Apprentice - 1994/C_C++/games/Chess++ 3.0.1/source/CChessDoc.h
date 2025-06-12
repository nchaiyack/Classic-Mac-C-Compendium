/****
 * CChessDoc.h
 *
 *	Document class for a typical application.
 *
 *	Copyright © 1993 Steven J. Bushell. All rights reserved.
 *
 ****/
 
#define	_H_CChessDoc			/* Include this file only once */
#include <CDocument.h>
#include <CApplication.h>
#include "CChessBoard.h"

struct CChessDoc : CDocument {

								/** Construction/Destruction **/
    
    void        IChessDoc(CApplication *aSupervisor, Boolean printable);
    void        Dispose(void);

	void		DoCommand(long theCommand);

    void        UpdateMenus(void);  

	void		NewFile(void);
	void		OpenFile(SFReply *macSFReply);
	void		BuildWindow(Handle theData);
	
									/** Filing **/
	Boolean		DoSave(void);
	Boolean		DoSaveAs(SFReply *macSFReply);
	void		DoRevert(void);

									/** Chess Doc Stuff **/
	void		SwapPlayers(void);
	void		DoChessOptionsDialog(void);
	void		ProviderChanged( CCollaborator *aProvider, long reason, void* info);
};
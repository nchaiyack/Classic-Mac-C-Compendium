/****
 * CHyperCuberDoc.h
 *
 *	Document class for a typical application.
 *
 ****/

#define	_H_CHyperCuberDoc			/* Include this file only once */
#include <CDocument.h>
#include <CApplication.h> 
#include "CGraphic.h"

CLASS CScrollBar;

struct CHyperCuberDoc : public CDocument {

	CGraphic		*graphic;					//  The graphic for this document

	CList			*controls_directors;		//  List of controls directors for this document.
												//    the nth item in this list is the director
												//    for the n-D controls.

	Boolean			fFullScreen;				//  TRUE if this window is a full-screen window
	Rect			normal_window_size;			//  size of window when it isn't full-screen	

  public:
    
    virtual void        IHyperCuberDoc(CApplication *aSupervisor, Boolean printable);
    virtual void        Dispose(void);

	virtual void		DoCommand(long theCommand);

    virtual void        UpdateMenus(void);  
	
	virtual void		NewFile(void);
	virtual void		NewFileFromTEXT(short TEXT_id);
	virtual void		NewNCube(long dimension);
	virtual void		OpenFile(SFReply *macSFReply);
	virtual void		OpenFileFromFilename(char *filename);
	virtual void		BuildGraphicsWindow(void);
	
	virtual void 		Activate(void);
	virtual void 		Deactivate(void);

	virtual void 		Suspend(void);
	virtual void 		Resume(void);

	virtual void		DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);
	virtual void		DoAutoKey(char theChar, Byte keyCode, EventRecord *macEvent);

	virtual void		CreatePICT(PicHandle& hypercube_pict);
	virtual Boolean		DoSaveAs(SFReply *macSFReply);

};
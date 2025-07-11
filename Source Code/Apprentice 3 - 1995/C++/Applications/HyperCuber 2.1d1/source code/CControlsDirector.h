//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CControlsDirector.h
//
// This file contains the interface to the controls window director.
//___________________________________________________________________________

#include <CDirector.h>
#include "CHyperScrollBar.h"

class CHyperCuberPane;

class CControlsDirector : public CDirector
	{

	CHyperCuberPane	*graphic_pane;				//  Pane containing the object these bars control
	long			dimension;					//  Dimension of the graphic to control

	CHyperScrollBar	perspective_bar;			//  The scroll bar which controls perspective
	CPtrArray<CHyperScrollBar> angle_bars;		//  List of angle scroll bars

	void	BuildWindow(void);
	void	SetupScrollBar(short voffset, char *title, CHyperScrollBar *bar,
								long angle_num,
									short min, short max, short value,
										Boolean wrap);

  public:

	virtual void	IControlsDirector(CDirectorOwner *aSupervisor,
										CHyperCuberPane *pane, long dimension);

	virtual void	ProviderChanged(CCollaborator *provider, long reason, void *info);
	virtual void	OffsetScrollBar(long angle, long offset);
	
	virtual void	ShowWindow(void);
	virtual void	HideWindow(void);

	};

/*************************************************************************************

 CStringEditList.h
	
		Interface for CStringEditList
	
	SUPERCLASS = CScrollList
	
		� 1992 Dave Harkness

*************************************************************************************/


#define _H_CStringEditList

#include "CScrollList.h"
#include "CEditString.h"


class CStringEditList : public CScrollList
{
public:

	void			IStringEditList( CView *anEnclosure, CBureaucrat *aSupervisor,
									 short aWidth, short aHeight,
									 short aHEncl, short aVEncl,
									 SizingOption aHSizing, SizingOption aVSizing,
									 short fOptions);
	
			// Commands
	
	virtual void	DoInsertCell( short beforeCell);

private:

	CEditString		*itsEditString;				// Pane for editing the string
	
			// Editing
	
	virtual void	BeginEditing( void);
	virtual void	SetupCellData( void);
	virtual void	RetrieveCellData( void);
	virtual void	DoneEditing( void);
	
			// Drawing
	
	virtual void	DrawCell( short theCell, Rect *cellRect);
	
			// Notification
	
	virtual void	ProviderChanged( CCollaborator *aProvider, long reason, void* info);

};

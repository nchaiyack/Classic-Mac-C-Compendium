/*************************************************************************************

 CEditString.h
	
		Interface for CEditString
	
	SUPERCLASS = CDialogText
	
		© 1992 Dave Harkness

*************************************************************************************/


#define _H_CEditString

#include <CDialogText.h>


class CEditString : public CDialogText
{
public:

	void		IEditString( CView *anEnclosure, CView *aSupervisor,
							 short aWidth, short aHeight, short aHEncl,  short aVEncl,
							 SizingOption aHSizing, SizingOption aVSizing,
							 short aLineWidth);
	
	virtual void		DoKeyDown( char theChar, Byte keyCode, EventRecord *macEvent);
	
	virtual Boolean		BecomeGopher( Boolean fBecoming);

protected:

	virtual void	MakeBorder( void);

};
	
	
/* Change protocol for CEditString		*/

enum
{
	editStringDoneEditing = dialogTextLastChange + 1,
	
	editStringLastChange = editStringDoneEditing

};

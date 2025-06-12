//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane
//|
//| This is the interface to a pane which displays a keyboard shortcut
//|______________________________________________________________________

#include <CEditText.h>
#include <CScrollBar.h>

class CKeyPane : public CEditText
	{
	
	short			code;
	
  public:

	virtual void	IKeyPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing, short line);

	virtual void	Draw(Rect *rect);
	virtual void	Highlight(void);

	virtual void	SetKey(short keycode);
	virtual short	GetKey(void);

	virtual void	DoKeyDown(char the_char, Byte key_code, EventRecord *event);
	
	virtual void	DoClick(Point hitPt, short modifierKeys, long when);

	virtual void	Activate(void);
	virtual void	Deactivate(void);

	};


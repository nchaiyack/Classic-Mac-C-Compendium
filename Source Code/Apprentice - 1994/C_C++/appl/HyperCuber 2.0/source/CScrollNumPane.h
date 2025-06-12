//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CScrollNumPane
//|
//| This is the interface to  a pane containing a number which reflects
//| the setting of a scroll bar
//|______________________________________________________________________

#include <CEditText.h>
#include <CScrollBar.h>

class CScrollNumPane : public CEditText
	{
	
	short 	num_offset;
	
  public:

	void	IScrollNumPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aWidth,
								short aHeight, short aHEncl, short aVEncl, SizingOption	aHSizing,
								SizingOption aVSizing, short line, CScrollBar *scroll_bar,
								short offset);

	void	CScrollNumPane::Draw(Rect *the_rect);

	void	ProviderChanged(CCollaborator *provider, long reason, void *info);

	};

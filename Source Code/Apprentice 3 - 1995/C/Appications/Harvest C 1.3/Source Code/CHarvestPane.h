/****
 * CHarvestPane.h
 *
 *	Pane class for a typical application.
 *
 ****/

#define _H_CHarvestPane			/* Include this file only once */
#include <CTable.h>

class CScrollPane;
class CTableLabels;

struct CHarvestPane : CTable {
									/** Contruction/Destruction **/
	void		IHarvestPane(CScrollPane *anEnclosure, CBureaucrat *aSupervisor);

	virtual void GetCellText( Cell aCell, short availableWidth, StringPtr itsText);
	virtual void	Draw( Rect *area);
};
	

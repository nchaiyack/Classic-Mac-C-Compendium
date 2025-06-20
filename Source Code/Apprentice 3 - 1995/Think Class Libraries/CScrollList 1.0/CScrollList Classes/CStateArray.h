/*************************************************************************************

 CStateArray.h
	
		Interface for CStateArray
	
	SUPERCLASS = CArray
	
		� 1992 Dave Harkness

*************************************************************************************/


#define _H_CStateArray

#include <CArray.h>


class CStateArray : public CArray
{
public:

	void			IStateArray( long howMany, short fState);
	
			// State Access
	
	virtual void	SetState( long index, short fState);
	virtual void	ToggleState( long index);
	virtual short	GetState( long index);
	virtual void	SetAllStates( short fState);

};

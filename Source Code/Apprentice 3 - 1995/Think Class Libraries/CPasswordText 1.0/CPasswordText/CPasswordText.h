#pragma once

#include <CDialogText.h>

class CPasswordText : public CDialogText
{
	public:
	
		void IPasswordText
			( CView *anEnclosure
			, CView *aSupervisor
			, short aWidth
			, short aHeight
			, short aHEncl
			, short aVEncl
			, SizingOption aHSizing
			, SizingOption aVSizing
			, short aLineWidth );

		virtual void IViewTemp
			( CView *anEnclosure
			, CBureaucrat *aSupervisor
			, Ptr viewData );
			
	protected:
	
		virtual void InstallTEHooks( void );

};
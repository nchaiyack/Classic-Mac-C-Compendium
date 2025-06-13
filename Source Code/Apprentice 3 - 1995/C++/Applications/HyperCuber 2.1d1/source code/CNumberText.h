//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText.h
//|
//| This is the interface for the CNumberText class.  CNumberText
//| implements an element in a dialog which accepts only numbers.
//|_______________________________________________________________

#pragma once

#include "CDialogText.h"

class CNumberText : public CDialogText
{
public:

	void 	INumberText( CView *anEnclosure, CView *aSupervisor,
					short aWidth, short aHeight,
					short aHEncl, short aVEncl,
					SizingOption aHSizing, SizingOption aVSizing,
					short aLineWidth);
					
	virtual void 	IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData);
	virtual void	SpecifyRange(double aMinimum, double aMaximum);
	virtual void	SpecifyDefaultValue(double aDefaultValue);
	
	virtual void	SetValue(double aValue);
	virtual double	GetValue(void);	

	virtual Boolean Validate(void);	

protected:

	double 	minValue;		// minimum valid value
	double 	maxValue;		// maximum valid value
	double	defaultValue;	// default value if text empty. If 
							// CDialogText::isRequired is FALSE, then
							// empty text is considered valid and default
							// value is returned by GetValue.
	Boolean showRangeOnErr; // If TRUE, validation error displays allow
							// range in the alert
							
	virtual void ConvertToNumber(double *intValue, Boolean *valid);

};

	// indices into validation error STR# resource for CDoubleText text

#define		validateIntFailed		3	// text not number or out of range
#define		validateIntRangeFailed	4	// same, but display allowed range in alert

typedef struct tNumberTextTemp
{
	tDialogTextTemp	dlgTextTmp;
	double			 minValue;
	double			 maxValue;
	double			 defaultValue;
	short			 showRangeOnErr;
	
} tNumberTextTemp, *tNumberTextTempP;
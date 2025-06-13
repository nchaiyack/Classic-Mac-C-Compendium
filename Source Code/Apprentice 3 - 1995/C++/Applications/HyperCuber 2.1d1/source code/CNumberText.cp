//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText.cp
//|
//| This implements an element in a dialog which accepts only
//| numbers.  The code used for this class is heavily based on
//| CIntegerText.
//|____________________________________________________________

#include "CNumberText.h"

#include <Global.h>
#include <TBUtilities.h>
#include <SANE.h>
#include <Packages.h>

#include <float.h>
#include <stdio.h>



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::INumberText
//|
//| Purpose: Initialize the CNumberText object.
//|
//| Parameters: passed to superclass
//|______________________________________________________________________________

void CNumberText::INumberText(CView *anEnclosure, CView *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing,
							short aLineWidth)
{
	CDialogText::IDialogText( anEnclosure, aSupervisor,
					aWidth, aHeight, aHEncl, aVEncl,
					aHSizing, aVSizing, aLineWidth);		//  Initialize superclass
					
	minValue = -DBL_MAX;								//  Default minimum = -DLB_MAX
	maxValue = DBL_MAX;									//  Default maximum = DBLMAX
	defaultValue = 0;									//  Default default value = 0
	isRequired = TRUE;									//  Field is required by default
	showRangeOnErr = FALSE;								//  Show allowable range if an error
														//   occurs, by default.

}	//==== CNumberText::INumberText() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::IViewTemp
//|
//| Purpose: Initialize the CNumberText object from a resource template.
//|
//| Parameters: passed to superclass
//|______________________________________________________________________________

void CNumberText::IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData)
{
	tNumberTextTempP data = (tNumberTextTempP) viewData;	//  Get copy of pointer to data
	
	inherited::IViewTemp(anEnclosure, aSupervisor,
							viewData);					//  Initialize superclass
	
	minValue = data->minValue;							//  Get minimum
	maxValue = data->maxValue;							//  Get maximum
	
	// As a special case, we interpret that
	// if minValue == maxValue, the intent is to allow all numbers
	
	if (minValue == maxValue)
	{
		minValue = -DBL_MAX;
		maxValue = DBL_MAX;
	}
	
	defaultValue = data->defaultValue;					//  Get default
	showRangeOnErr = data->showRangeOnErr != 0;			//  Get whether to show range on error

}	//==== CNumberText::IViewTemp() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::SpecifyRange
//|
//| Purpose: Specify the allowed range of allowable values.
//|
//| Parameters: aMinimum: minimum allowable value
//|             aMaximum: maximum allowable value
//|______________________________________________________________________________

void CNumberText::SpecifyRange(double aMinimum, double aMaximum)
{

	ASSERT( aMaximum > aMinimum);				//  Make sure max > min
	
	minValue = aMinimum;						//  Save minimum
	maxValue = aMaximum;						//  Save maximum
	
	// if range is (-DLB_MAX, DLB_MAX) then any numerical entry is
	// allowed, so don't display the acceptable range in the error alert
	
	showRangeOnErr = !((aMinimum == -DBL_MAX) && (aMaximum == DBL_MAX));
	
}	//==== CNumberText::SpecifyRange() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::SpecifyDefaultValue
//|
//| Purpose: Specify the default value.
//|
//| Parameters: aDefaultValue: default value
//|______________________________________________________________________________

void CNumberText::SpecifyDefaultValue(double aDefaultValue)
{

	defaultValue = aDefaultValue;
	isRequired = FALSE;
	
}	//==== CNumberText::SpecifyDefaultValue() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::SetValue
//|
//| Purpose: Specify the value of the dialog text item.
//|
//| Parameters: aValue: the new value
//|______________________________________________________________________________

void CNumberText::SetValue(double aValue)
{

	Str255 oldText;
	GetTextString( oldText);					//  Get the old number

	Str255 numStr;
	sprintf((char *) numStr, "%g", aValue);		//  Convert number to a pascal string
	CtoPstr((char *) numStr);
	
	SetTextString( numStr);						//  Set dialog text to new value
	
	if (!Validate())
		SetTextString( oldText);				//  If validation failed, restore old value
		
}	//==== CNumberText::SetValue() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::GetValue
//|
//| Purpose: Get the value of the dialog text item.  If the text is empty,
//|          the default value is returned
//|
//| Parameters: returns the new value
//|______________________________________________________________________________

double CNumberText::GetValue( void)
{
	double 	value;
	Boolean valid;
	
	ConvertToNumber(&value, &valid);
			
	return value;
	
}	//==== CNumberText::GetValue() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::Validate
//|
//| Purpose: Ensure that all the constraints have been met. If validation
//|          fails, the ReportInvalidText method is called to show an alert.
//|
//| Parameters: returns TRUE iff validation is successful
//|______________________________________________________________________________

Boolean CNumberText::Validate( void)
{
	double	value;
	Boolean valid;
	Str31	minText, maxText;
	
	valid = inherited::Validate();					//  Check if it's valid according to
													//  the superclass
	
	if (valid)
	{
	
		ConvertToNumber( &value, &valid);			//  Get the value of the number, and
													//   check validity

		if (valid)
		{
			if ((value < minValue) ||
				(value > maxValue))
				valid = FALSE;						//  Check against bounds
		}
	
		if (!valid)
		{
			if (showRangeOnErr)
			{
				sprintf((char *) minText, "%d", minValue);	//  Convert min to pascal string
				CtoPstr((char *) minText);
				sprintf((char *) maxText, "%d", maxValue);	//  Convert max to pascal string
				CtoPstr((char *) maxText);
				ParamText( NULL, minText, maxText, NULL);
				ReportInvalidText( validateIntRangeFailed);
			}
			else
				ReportInvalidText( validateIntFailed);
		}
	}
	return valid;
	
}	//==== CNumberText::Validate() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNumberText::ConvertToNumber
//|
//| Purpose: Protected method to convert the text to a double. Returns TRUE if
//|          the text was a valid number.
//|
//| Parameters: numberValue: receives the value of the number
//|             valid:       receives TRUE if number is valid
//|______________________________________________________________________________

void CNumberText::ConvertToNumber(double *numberValue, Boolean *valid)
{

	Str255	str;
	short	index, length;
	decimal dec;
	Boolean	validPrefix = 0;
	long double val;
	GetTextString( str);
		
	length = Length(str);
	if (length == 0)
	{
		*valid = !isRequired;
		*numberValue = defaultValue;
	}
	else
	{		
		index = 1;
		
		str2dec((const char*) str, &index, &dec, &validPrefix);
		
		// if validPrefix is TRUE and index is past the last char,
		// then the string is a valid numeric string. If the
		// exp field is zero, then it is a valid integer
		
		*valid = validPrefix && (index > length);

	}
	
}	//==== CNumberText::ConvertToNumber() ====\\
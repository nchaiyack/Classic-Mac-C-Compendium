#include "ep.const.h"
#include "ep.extern.h"

cWindowDataUpate(Resized) /************************************************************/
Boolean	Resized;
{
char	s[64];

	MoveTo(75,82);
	NumToString(GetCtlValue(latSB),s);
	EraseRect(&latDataRect);
	DrawString(s);

	MoveTo(82,162);
	NumToString(GetCtlValue(lonSB),s);
	EraseRect(&lonDataRect);
	DrawString(s);

	MoveTo(73,242);
	NumToString(GetCtlValue(altSB)*(long)altScale,s);
	EraseRect(&altDataRect);
	DrawString(s);
}

cUpdate (Resized) /************************************************************/
Boolean	Resized;
{
GrafPtr	thePort;

	GetPort (&thePort);

	PlotIcon(&iconRect,iconHandle);
	DrawControls(thePort);

	MoveTo(0,51);	/* draw nice lines :-) */
	LineTo(175,51);
	MoveTo(0,54);
	LineTo(175,54);

	TextFont(0);
	MoveTo(60,30);
	DrawString("\p�EarthPlot v3");
	MoveTo(9,82);
	DrawString("\pLatitude:");
	MoveTo(9,162);
	DrawString("\pLongitude:");
	MoveTo(9,242);
	DrawString("\pAltitude:");
	
	cWindowDataUpate();
}

cClose() /************************************************************/
{
	SkelWhoa ();
}

cClobber() /************************************************************/
{
GrafPtr	thePort;

	GetPort (&thePort);				/* grafport of window to dispose of */
	DisposeWindow ((WindowPtr) thePort);
}

cActivate() /************************************************************/
{
}

pascal void Track(Control,partCode)
ControlHandle	Control;
int				partCode;
{
int		i,step;
long	wait;
long	waited;
char	s[128];

	wait = 5;  /* ticks */

	if (partCode == 0) return;
	
	switch (partCode) {
	case inUpButton:
		step = -1;
		break;
	case inDownButton:
		step = 1;
		break;
	case inPageUp:
		step = -10;
		break;
	case inPageDown:
		step = 10;
		break;
	}
	
	i = GetCtlValue(Control) + step;
	
	if (*Control == *latSB) {
		if (i > 90) i = 90;
		if (i <  0) i = 0;

		SetCtlValue(Control,i);

		EraseRect(&latDataRect);
		MoveTo(75,82);
		NumToString(GetCtlValue(latSB),s);
		DrawString(s);
		Delay(wait,&waited);
	}

	if (*Control == *lonSB) {
		if (i > 180) i = 180;
		if (i <   0) i = 0;

		SetCtlValue(Control,i);

		EraseRect(&lonDataRect);
		MoveTo(82,162);
		NumToString(GetCtlValue(lonSB),s);
		DrawString(s);
		Delay(wait,&waited);
	}

	if (*Control == *altSB) {
		if (i > 180) i = 180;
		if (i <   0) i = 0;

		SetCtlValue(Control,i);

		EraseRect(&altDataRect);
		MoveTo(73,242);
		NumToString(GetCtlValue(altSB)*(long)altScale,s);
		DrawString(s);
		Delay(wait,&waited);
	}
}

cMouse(thePoint,theTime,theMods) /************************************************************/
Point	thePoint;
long	theTime;
int		theMods;
{
ControlHandle	theControl;
int				partCode;
char			s[100];

	partCode = FindControl(thePoint,cWindow,&theControl);

	if (partCode) {
		switch (partCode) {
		case inCheckBox:
			partCode = TrackControl(theControl,thePoint,0L);
			if (*theControl == *northCheck) {
				SetCtlValue(northCheck,1);
				SetCtlValue(southCheck,0);
			}
			if (*theControl == *southCheck) {
				SetCtlValue(northCheck,0);
				SetCtlValue(southCheck,1);
			}
			if (*theControl == *eastCheck) {
				SetCtlValue(eastCheck,1);
				SetCtlValue(westCheck,0);
			}
			if (*theControl == *westCheck) {
				SetCtlValue(eastCheck,0);
				SetCtlValue(westCheck,1);
			}
			if (*theControl == *mileCheck) {
				SetCtlValue(mileCheck,1);
				SetCtlValue(kmCheck,0);
			}
			if (*theControl == *kmCheck) {
				SetCtlValue(mileCheck,0);
				SetCtlValue(kmCheck,1);
			}
			break;
		case inUpButton:
		case inDownButton:
		case inPageUp:
		case inPageDown:
			partCode = TrackControl(theControl,thePoint,Track);
			break;
		case inThumb:
			partCode = TrackControl(theControl,thePoint,0L);
			cWindowDataUpate(FALSE);
			break;
		}
	}
}

cKey() /************************************************************/
{
}

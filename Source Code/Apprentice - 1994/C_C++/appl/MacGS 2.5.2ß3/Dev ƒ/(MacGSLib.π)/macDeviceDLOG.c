/* Copyright (C) 1993 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

#include <stdio.h>
#include <stddef.h>
#include "gx.h"				/* for gx_bitmap; includes std.h */
#include "iref.h"
#include "gxdevice.h"
#include "state.h"
#include "gsstate.h"
#include "gmacres.h"
#include "gp_macui.h"


extern	gx_device  *gx_device_list[];

void	DoSetDeviceDialog (void);


enum
{
	iOKOutline = Cancel + 1,
	iOptions,

	__iFirstDeviceDITL__,
	iMacDisplay = __iFirstDeviceDITL__,
	iBitTimer,
	iGIFMono,
	iGIF8Bit,
	iPbm,
	iPbmRaw,
	iPgm,
	iPgmRaw,
	iPpm,
	iPpmRaw,

	__iLastDialogItem__
};


#define CALLBACK(procName, args)							\
{															\
	if (pGUIProcs /* != (GUIProcPtr) NULL */ &&				\
		pGUIProcs->procName /* != (void) (*) (...) NULL */)	\
	{														\
		callbackStatus = (*pGUIProcs->procName) args;		\
	}														\
}

static short		DoSetDeviceDialogPriv (void);

static GUIProcPtr	pGUIProcs = (GUIProcPtr) NULL;
static short		lastItem  = 0;
static short		ithDevice;
static char			callbackStatus;


	void
DoSetDeviceDialog (void)

{
	short	item;


	do
	{
		switch (item = DoSetDeviceDialogPriv ())
		{
			case iOptions:
			{
				CALLBACK (DoOptionsProc, (iDoDialogAction));
				break;
			}
		}

	} while (item != OK && item != Cancel);

	/*...Select the device...*/

	if (item == OK)
	{
		if (lastItem != ithDevice)
		{
			Str255	string;
			Str255	psString;


			NumToString ((long) (lastItem - __iFirstDeviceDITL__), string);
			GetIndString (psString, MACPSSTRS_RES_ID, iMacSetDevice);
			BlockMove ((Ptr) &psString[1], (Ptr) &string[string[0] + 1],
					   (Size) psString[0]);
			string[0] += psString[0];

			(void) doRunString (PtoCstr (string));
		}

		CALLBACK (DoOptionsProc, (iCommitAction));
	}
	else
	{
		CALLBACK (DoOptionsProc, (iRetractAction));
	}
}


#define SET_RADIO_BUTTON_VALUE(ithDevice, value)								\
{																				\
	if (__iFirstDeviceDITL__ <= ithDevice && ithDevice < __iLastDialogItem__)	\
	{																			\
		GetDItem (dialog, ithDevice, &iType, &hItem, &rect);					\
		if (iType & (ctrlItem | radCtrl))										\
			SetCtlValue ((ControlHandle) hItem, value);							\
	}																			\
}


#define SET_DEVICE_RADIO_BUTTON(ithDevice)										\
{																				\
	if (lastItem != ithDevice)													\
	{																			\
		if ((dev = gs_getdevice (ithDevice - __iFirstDeviceDITL__))				\
				/* != (gx_device *) NULL */)									\
		{																		\
			GUIProcPtr	pOldGUIProcs = pGUIProcs;								\
																				\
																				\
			callbackStatus = kNoOptions;										\
																				\
			pGUIProcs = FindGUIProcs (gs_devicename (dev));						\
																				\
			if (pGUIProcs /* != (GUIProcPtr) NULL */ &&							\
				pGUIProcs->DoOptionsProc /* != (GUIOptionsProc) NULL */)		\
			{																	\
				CALLBACK (DoOptionsProc, (iInitDialogAction));					\
			}																	\
																				\
			if (callbackStatus & kIsIllegalDrvr)								\
			{																	\
				pGUIProcs = pOldGUIProcs;										\
			}																	\
			else																\
			{																	\
				SET_RADIO_BUTTON_VALUE (lastItem , 0);							\
				SET_RADIO_BUTTON_VALUE (ithDevice, 1);							\
																				\
				lastItem = ithDevice;											\
																				\
				GetDItem (dialog, iOptions, &iType, &hItem, &rect);				\
				if (iType & (ctrlItem | btnCtrl))								\
				{																\
					short	hiliteCode = (callbackStatus & kHaveOptions)		\
											 ?   0								\
											 : 255;								\
																				\
																				\
					HiliteControl ((ControlHandle) hItem, hiliteCode);			\
				}																\
			}																	\
																				\
		}																		\
	}																			\
}


	short
DoSetDeviceDialogPriv (void)

{
	short		item   = Cancel;
	DialogPtr	dialog = GetNewDialog (MACSETDEVICEDLOG, (char *) NULL, (WindowPtr) -1);


	if (dialog /* != (DialogPtr) NULL */)
	{
		short		iType;
		Handle		hItem;
		Rect		rect;
		gx_device **pDeviceList;
		gx_device  *dev		   = (gx_device *) NULL;
		gx_device  *currDev	   = gs_currentdevice (igs);
		Boolean		fBreakLoop;


		pGUIProcs = (GUIProcPtr) NULL;
		lastItem  = 0;

		/*...Prep outline field...*/

		GetDItem (dialog, iOKOutline, &iType, &hItem, &rect);

		if (iType == userItem | itemDisable)
			SetDItem (dialog, iOKOutline, iType, (Handle) drawOutline, &rect);

		/*...Establish the default driver...*/

		for (pDeviceList = gx_device_list, ithDevice = __iFirstDeviceDITL__;
			 *pDeviceList /* != (gx_device *) NULL */;
			 pDeviceList++, ithDevice++)
		{
			if (*pDeviceList == currDev)
			{
				SET_DEVICE_RADIO_BUTTON (ithDevice);
				break;
			}
		}

		/*...Run the modal dialog...*/

		ShowWindow (dialog);

		for (fBreakLoop = FALSE; !fBreakLoop; )
		{
			ModalDialog ((ProcPtr) NULL, &item);

			switch (item)
			{
				case OK:
				case Cancel:
				case iOptions:
					fBreakLoop = TRUE;
					break;

				default:
				{
					SET_DEVICE_RADIO_BUTTON (item);
					break;
				}
			}
		}

		DisposDialog (dialog);
	}

	return item;
}

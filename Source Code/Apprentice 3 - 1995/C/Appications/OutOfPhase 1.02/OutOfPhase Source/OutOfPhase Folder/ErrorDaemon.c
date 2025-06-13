/* ErrorDaemon.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "ErrorDaemon.h"
#include "Memory.h"


struct ErrorDaemonRec
	{
		float						MaxClampValue;
	};


/* get a new error daemon */
ErrorDaemonRec*			NewErrorDaemon(void)
	{
		ErrorDaemonRec*		Daemon;

		Daemon = (ErrorDaemonRec*)AllocPtrCanFail(sizeof(ErrorDaemonRec),"ErrorDaemonRec");
		if (Daemon == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Daemon->MaxClampValue = 0;
		return Daemon;
	}


/* dispose of the error daemon */
void								DisposeErrorDaemon(ErrorDaemonRec* Daemon)
	{
		CheckPtrExistence(Daemon);
		ReleasePtr((char*)Daemon);
	}


/* report a new clamping value.  value should be positive and greater than 1 */
void								ErrorDaemonReportClamping(ErrorDaemonRec* Daemon, float ClampedValue)
	{
		CheckPtrExistence(Daemon);
		ERROR(ClampedValue < 0,PRERR(AllowResume,
			"ErrorDaemonReportClamping:  passed value is less than zero"));
		ERROR(ClampedValue <= 1,PRERR(AllowResume,
			"ErrorDaemonReportClamping:  passed value is not greater than one"));
		if (Daemon->MaxClampValue < ClampedValue)
			{
				Daemon->MaxClampValue = ClampedValue;
			}
	}


/* return True if clamping occurred */
MyBoolean						ErrorDaemonDidClampingOccur(ErrorDaemonRec* Daemon)
	{
		CheckPtrExistence(Daemon);
		return (Daemon->MaxClampValue > 1);
	}


/* obtain the maximum clamping value */
float								ErrorDaemonGetMaxClamping(ErrorDaemonRec* Daemon)
	{
		CheckPtrExistence(Daemon);
		return Daemon->MaxClampValue;
	}

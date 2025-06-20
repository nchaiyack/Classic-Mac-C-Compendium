/******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DSUtils.h
**
**   Description:	header w/protos for DSUtils
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	12/09/91			LDR		Added protos for new routines
**	11/24/91			LDR		original version
**
******************************************************************************/

#pragma once

#ifndef	__DSUTILS_H__
#define	__DSUTILS_H__

#ifndef THINK_C
	#include <Types.h>
	#include <Memory.h>
	#include <QuickDraw.h>
	#include <OSUtils.h>
	#include <ToolUtils.h>
	#include <Menus.h>
	#include <Packages.h>
	#include <Traps.h>
	#include <Files.h>
	#include <Resources.h>
#endif

#include <Aliases.h>
#include <Processes.h>
#include <PPCToolbox.h>
#include <DSGlobals.h>

void CenterAlert ( short theID );
void ErrorAlert ( short stringListID, short stringIndexID, short errorID );

void GetAppName(Str255 appName);
void GetAppFSSpec(FSSpec *appSpec);

OSErr GetTargetFromSelf (AEAddressDesc *targetDesc);
OSErr GetTargetFromSignature (OSType processSig, AEAddressDesc *targetDesc);
OSErr GetTargetFromBrowser (Str255 promptStr, AEAddressDesc *targetDesc);

void _SendDocsToSelf (AEDescList *aliasList);
void SendODOCToSelf (FSSpec *theFileSpec);
void SendQuitToSelf (void);

#endif	__DSUTILS_H__

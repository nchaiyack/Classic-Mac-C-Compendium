// ===========================================================================
//	CTestDialogs.h
//
//	This file contains the header information for the example code to
//	demonstrate the functionality of the CModalProgress class.
//
//	Copyright 1994 Alysoft Solutions. All rights reserved.
//
// ===========================================================================

#pragma once

#include "CModalProgress.h"

void TestDialog1(void) ;
void TestDialog2(void) ;
void TestDialog3(void) ;
void TestDialog4(void) ;

/*
 *	Test 1 Just a simple example, has no sub-classing, just some res ids
 */

#define	kTest1DialogResID				128
#define	kTest1ProgressBarItem			4

/*
 *	Test 2 creates a sub-class of the main progress class
 */

enum
{
	kTest2State1				= 65,
	kTest2State1StateSpace					= 100,
	kTest2State2				= 80,
	kTest2State2StateSpace					= 100,
	kTest2State3				= 100,
	kTest2State3StateSpace					= 100
} ;

#define	kTest2State1String				"\pCompiling File: TestFile.cp"
#define kTest2State2String				"\pLinking: TestApplication"
#define	kTest2State3String				"\pCopying Resources"

#define	kTest2DialogResID				129
#define	kTest2ProgressBarItem			4
#define	kTest2ItemText					5

class CDialog2 : public CModalProgress
{
	public:
		void SetCurrentState(float statePercent) ;
	
} ;

/*
 *	Test 3 uses the infinite progress bar in a search type dialog
 */

#define	kTest3DialogResID				130
#define	kTest3InfiniteBarItem			4

/*
 * 	Test 4 uses lots of states, more customization of the base class with
 *	the addition of an extra method to set a file name text field.
 */

enum
{
	kTest4ConnectState			= 5,
	kTest4LoginState			= 10,
	kTest4DownloadState			= 70,
	kTest4DownloadStateSpace				= 2000,
	kTest4File1StateSpace					= 500,
	kTest4File2StateSpace					= 1000,
	kTest4File3StateSpace					= 500,
	kTest4SetParamsState		= 80,
	kTest4LogoffState			= 85,
	kTest4DisconnectState		= 90,
	kTest4CleanupState			= 100,
	kTest4StandardStateSpace				= 500
} ;

#define	kTest4ConnectStateString		"\pConnecting"
#define	kTest4LoginStateString			"\pLogin"
#define	kTest4DownloadStateString		"\pDownlad Files"
#define	kTest4DownloadFileNone			"\p"
#define	kTest4DownloadFile1				"\puserparams.txt"
#define	kTest4DownloadFile2				"\pmainscript.txt"
#define	kTest4DownloadFile3				"\pconfig.txt"
#define	kTest4SetParamsStateString		"\pSetting Parameters"
#define	kTest4LogoffStateString			"\pLogoff"
#define	kTest4DisconnectStateString		"\pDisconnecting"
#define	kTest4CleanupStateString		"\pCleanup"

#define	kTest4DialogResID				131
#define	kTest4ProgressBarItem			4
#define	kTest4StateTextItem				5
#define	kTest4FileNameTextItem			6
#define	kTest4PercentTextItem			10

class CDialog4 : public CModalProgress
{
	public:
		void SetCurrentState(float statePercent) ;
		void SetFileName(Str255 filename) ;
} ;

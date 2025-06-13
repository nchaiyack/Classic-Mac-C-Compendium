/* Globals - Contains declarations of global nature                           */

#ifndef __GLOBALS__
#define __GLOBALS__

#if qDebug
#include <stdio.h>
#include <iostream.h>
#endif


const void *kDummyLink = (const void *) -1;

const unsigned char chBell = 7;
const unsigned char chLineFeed = 10;

const short puFont = 1002;


// ---------- File Types

const unsigned long kSignature = ' %|g';
const unsigned long kFileType = 'MUD ';

// ---------- Alert and dialog IDs

const short phGetDirID = 1000;
const short phCmdErrID = 1001;
const short phNoCommID = 1002;
const short phNoToolsID = 1003;
const short phPurgedID = 1004;
const short phS7GetDirID = 1005;
const short phConvertID = 1006;
const short phOpenLogID = 1007;
const short phS7OpenLogID = 1008;
const short phUnavailID = 1009;
const short phNoMTPDirID = 1010;

// ---------- View Resource IDs

const short kMUDWindID = 1000;
const short kPrefsID = 1001;
const short kCommID = 1002;
const short kConfigTCPID = 1003;
const short kSetupID = 1004;
const short kUploadID = 1005;
const short kDownloadID = 1006;
const short kUpdateID = 1007;
const short kCmdScrlID = 1008;
const short kMacrosID = 1009;
const short kTypeKeyID = 1010;
const short kPasswordID = 1011;

// ---------- Menu items and internal commands

const short cConfigure = 1001;
const short cPrefs = 1002;
const short cConnect = 1003;
const short cBreak = 1004;
const short cComm = 1005;
const short cInterrupt = 1006;
const short cUpdate = 1007;
const short cUpdateAll = 1008;
const short cDownload = 1009;
const short cSetup = 1010;
const short cSendFile = 1011;
const short cUseCTB = 1012;
const short cUpload = 1013;
const short cLogFile = 1014;
const short cMacros = 1015;

const short cReturnKey = 2000;
const short cUpKey = 2001;
const short cDownKey = 2002;


// ---------- String resources

const short kMUDStrings = 1002;
const short kmsConnect = 1;
const short kmsDisconnect = 2;
const short kmsTCPSetup = 3;
const short kmsToolSetup = 4;
const short kmsUpdate = 5;
const short kmsEd = 6;
const short kmsWrite = 7;
const short kmsDelete = 8;
const short kmsAppend = 9;
const short kmsStop = 10;
const short kmsQuit = 11;
const short kmsEdPrompt = 12;
const short kmsPrompt = 13;
const short kmsFirst = 14;
const short kmsNext = 15;
const short kmsAgain = 16;
const short kmsFirstBlk = 17;
const short kmsNextBlk = 18;
const short kmsAgainBlk = 19;
const short kmsUnrecognized = 20;
const short kmsCreator = 21;
const short kmsHost = 22;
const short kmsOpenLog = 23;
const short kmsCloseLog = 24;
const short kmsLogName = 25;
const short kmsSaveLog = 26;
const short kmsShift = 27;
const short kmsOption = 28;
const short kmsControl = 29;
const short kmsDEL = 30;
const short kmsClear = 31;
const short kmsEscape = 32;
const short kmsF1 = 33;
const short kmsF2 = 34;
const short kmsF3 = 35;
const short kmsF4 = 36;
const short kmsF5 = 37;
const short kmsF6 = 38;
const short kmsF7 = 39;
const short kmsF8 = 40;
const short kmsF9 = 41;
const short kmsF10 = 42;
const short kmsF11 = 43;
const short kmsF12 = 44;
const short kmsF13 = 45;
const short kmsF14 = 46;
const short kmsF15 = 47;
const short kmsFwdDel = 48;
const short kmsOff = 49;

const short kKeyStrings = 1003;
const short kCmdStrings = 1004;

// ---------- Errors

const short errNoTools = -25001;
const short errInitCM = -25002;
const short errDocFormat = -25003;
const short errNetFail = -25004;
const short errProtocol = -25005;
const short errNetUnreach = -25006;
const short errHostUnreach = -25007;
const short errProtUnreach = -25008;
const short errPortUnreach = -25009;
const short errCMGeneric = -25010;
const short errCMRejected = -25011;
const short errCMFailed = -25012;
const short errCMTimeOut = -25013;
const short errCMNotOpen = -25014;
const short errCMNotClosed = -25015;
const short errCMNoRequest = -25016;
const short errCMNotSup = -25017;
const short errCMNoTools = -25018;
const short errOpenTimeout = -25019;

// ---------- Global variables

short gTCPRef;
short gDefVRefNum;
long gDefDirID;

Boolean gCTBAvailable;
Boolean gCTBToolsFound;
Boolean gTCPAvailable;

Boolean gDisableMenus;
Boolean gStop;

#endif

#ifndef bscoh
#define bscoh
// Alternativ: #pragma once
#ifdef apple
#ifdef modus_mcp

#include <ctype.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <size_t.h>
#include <stat.h>
#include <stdarg.h>
#include <time.h>



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <GestaltEqu.h>
#include <Files.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <types.h>

#ifdef AppleEventsANSI		// In AppleEvents.h werden Files included, die Enumeration Constants
#include <AppleEventsANSI.h>	// Vereinbaren, die nicht als Int durchgehen => Schwierigkeiten mit
#else 				// ANSI-Einstellung Enumeration Constants are always int!
#include <AppleEvents.h>	// Falls AppleEventsANSI definiert ist wird das umgangen, es werden
#endif				// modifizierte Header included (AppleEventsANSI, EPPCANSI und ProcessesANSI)
				// KEINE GARANTIE F�R DAS FUNKTIONIEREN DIESER HEADER!!!!!!!!!!!!!!!!!!!!!!!!



/***************************************************************************/
/*****   Include-Datei f�r SETI: Mac Daten					           *****/
/*****   f�r MCP und Inter-Modus                                       *****/
/*****   Autor:    Andreas Amoroso                                     *****/
/*****   Datum:    11.8.1993                                           *****/
/***************************************************************************/


// Resource-IDs for Alerts
// N.B.:
#define rNoPDocAlert	128
#define rCannotOpenFile 129
#define rPathTooLong	130
#define rNoOAppAlert	131		//Erg�nzt von Amoroso, 31.8.93, f�r den Fall das beide Modi doch zusammen gehen!
#define dOKButton	  1


// Limit of Files to open. Change, if necessary!

#define maxFiles	  8


//PROTOTYPES: used in this file

char **GetProgParams(int *);					// Retrieves list of files from ODoc-Event and puts it to argv
//void	InitToolbox(void);						// Installs the Toolbox
static void InitTBandCON(void);					// Installs the Toolbox compatible with the Console Package
Boolean AppleEventsInstalled (void);			// Tests if AppleEvents available
void testlength(char *);						// Tests if full pathname is too long
Boolean GetQuitReq(int);						// Tests if QuitApplication-Event was received
void GetFullPath(long DirID, short vRefNum, StringPtr fullPath);
												// Assembles the full pathname of a file under System-7
void DoError(OSErr);							// Too complicated to describe!!
int GetFileInfo(char *, unsigned long *, unsigned long *);	
												// Get Type and Creator
int SetFileInfo(char *, unsigned long, unsigned long);	
												// Manipulate Filename, Type and Creator
int GetProgName(char *);						// Set String with ProgName:Creator
												// ":" Kann auf dem Mac nicht f�r Dateinamen verwendet werden!!!
int GetStartMods(char *, unsigned short);		// Set String with ":modifiers csaok" if pressed while startup
												// Control,Shift,Alpha Lock,Option,KControl...
int TestStartMods(unsigned short);	
int checkType(FILE *, char *, int *,  int);
												// Test file on being .con or .dat
int swapArgv(char *, char *);					// Swap two argvs




// AppleEvent-Handlers

pascal OSErr  MyHandleODoc(AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);	// AppleEvent-Handlers
pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);	// AppleEvent-Handlers
pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);	// AppleEvent-Handlers
pascal OSErr  MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);	// AppleEvent-Handlers
OSErr MyGotRequiredParams (AppleEvent *theAppleEvent);											// MAC, checks for AE PARAMS 

#endif //modus_mcp
#endif //apple
#endif //ifndef bscoh
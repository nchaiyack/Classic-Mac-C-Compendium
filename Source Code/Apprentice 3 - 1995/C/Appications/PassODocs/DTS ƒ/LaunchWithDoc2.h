/*
	LaunchWithDoc2.h
	
	Header file for LaunchWithDoc2.c
	
*/

#pragma once

#ifndef __H_LaunchWithDoc2__
#define __H_LaunchWithDoc2__

#ifdef __cplusplus
extern "C" {
#endif

// prototypes

OSErr OpenSpecifiedDocument(const FSSpec * documentFSSpecPtr);
OSErr FindApplicationFromDocument(const FSSpec * documentFSSpecPtr,
	FSSpecPtr applicationFSSpecPtr);
OSErr BuildOpenDocumentEvent(ProcessSerialNumber *targetPSN, 
			const FSSpec *theSpecArray, const short numOfSpecs, AppleEvent *odocAppleEvent);
OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber *targetPSN,
			const FSSpec *theSpecArray, const short numOfSpecs);
OSErr LaunchApplicationWithDocument(const FSSpec *applicationFSSpecPtr,
			const FSSpec *theSpecArray, const short numOfSpecs);

#ifdef __cplusplus
}
#endif

#endif /* __H_LaunchWithDoc2__ */



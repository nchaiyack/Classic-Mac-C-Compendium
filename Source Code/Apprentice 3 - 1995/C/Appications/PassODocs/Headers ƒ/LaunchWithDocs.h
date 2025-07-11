/*
	LaunchWithDocs.h
	
	Header file for LaunchWithDocs.c
	
*/

#pragma once

#ifndef __H_LaunchWithDocs__
#define __H_LaunchWithDocs__

#ifdef __cplusplus
extern "C" {
#endif

OSErr OpenSpecifiedDocumentByCreator(const FSSpec * docSpec);
OSErr OpenSpecifiedDocumentWithCreator(const FSSpec * docSpec,OSType creator);
Boolean CreatorToProcessID(OSType creator,ProcessSerialNumber* psn);
OSErr LaunchApplicationWithDocument(const FSSpec* appSpec,const FSSpec* docSpec);
OSErr LaunchApplicationWithDocuments(const FSSpec* appSpec,const FSSpec* specArr,
	const short numSpecs);
OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber* target,const FSSpec* specArr,
	const short numSpecs);
OSErr FindApplicationFromDocument(const FSSpec* docSpec,FSSpecPtr appSpec);
OSErr FindApplicationFromCreator(OSType creator,FSSpecPtr appSpec);
OSErr BuildOpenDocumentEvent(ProcessSerialNumber* target,const FSSpec* specArr,
	const short numSpecs,AppleEvent* odoc);

#ifdef __cplusplus
}
#endif

#endif /* __H_LaunchWithDocs__ */






/*
 * JPartialResources.h
 *
 * Jamie's partial resource calls that work under any system.
 * © Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 *
 */



/******************************/

extern Boolean partialResourceCallsAvailable;

/******************************/



void jReadPartialResource(Handle theResource, long offset, void *buffer, long count);
void jWritePartialResource(Handle theResource, long offset, void *buffer, long count);
void jSetResourceSize(Handle theResource, long size);

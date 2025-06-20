/* ==========================================

	fss_utl2.proto.h
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========================================== */

OSErr fss_Create( int gotFSSpec, const FSSpec *spec, OSType creator, OSType type, ScriptCode tag );
OSErr fss_CreateResFile( int gotFSSpec, const FSSpec *spec, OSType creator, OSType type, ScriptCode tag);
OSErr fss_Delete(int gotFSSpec, const FSSpec *spec);

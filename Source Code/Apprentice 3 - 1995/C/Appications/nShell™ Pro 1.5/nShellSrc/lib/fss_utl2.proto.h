/* ==========================================

	fss_utl2.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
		
   ========================================== */

OSErr fss_Create( int gotFSSpec, const FSSpec *spec, OSType creator, OSType type, ScriptCode tag );
OSErr fss_CreateResFile( int gotFSSpec, const FSSpec *spec, OSType creator, OSType type, ScriptCode tag);
OSErr fss_Delete(int gotFSSpec, const FSSpec *spec);

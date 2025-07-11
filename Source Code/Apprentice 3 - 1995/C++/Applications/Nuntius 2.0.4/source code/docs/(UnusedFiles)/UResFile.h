// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UResFile.h

#define __URESFILE__

class TResFile : public TFile
{
	public:
		// errorchecking is done automatically in these funcs
		Handle GetResource(ResType theType, short theID);
		Handle GetRes(ResType theType, ConstCStr255Param name);
		void AddResource(void *theResource, ResType theType, short theID, ConstCStr255Param name);
		void AddRes(void *theResource, ResType theType, ConstCStr255Param name);
		void ReleaseResource(void *theResource);
		void DetachResource(void *theResource);
		
		void UpdateRes(void *theResource);
		// makes ChangedResource and WriteResource

		Boolean GetResBool(ResType theType, ConstCStr255Param name, Handle &H);
		Boolean GetResourceBool(ResType theType, short theID, Handle &H);
		// returns false if resource doesn't exist, does errorchecking

		Boolean ResExist(ResType theType, short theID);
		Boolean NamedResExist(ResType theType, ConstCStr255Param name);
		// does not signal if the res doesn't exist (but otherwise)

		void RemoveRes(ResType theType, ConstCStr255Param name);
		// does not signal if the res doesn't exist (but otherwise)

		void UpdateFile();
		// calls UpdateResFile
		
		short UniqueID(ResType theType);
		
		// these routines disposes the res-handles
		void RemoveAllResources();
		void RemoveResHandle(Handle H);
		void RemoveResource(ResType theType, short theID);
		
		pascal OSErr CloseRsrcFork(); // flushes volume too

		TResFile();
		pascal void Initialize();
		pascal void IResFile(OSType itsFileType,
							  OSType itsCreator,
							  Boolean usesDataFork,
							  ResourceUsage usesRsrcFork,
							  Boolean keepsDataOpen,
							  Boolean keepsRsrcOpen);
		pascal void Free();
};

/* CSourceFile.h */
#pragma once

#include	"CObject.h"
#include	"stdio.h"

class CDataFile;
class CHarvestOptions;
class CArray;
class CErrorLog;

#define H_SourceFile 1
#define H_LibraryFile 2
#define H_ResourceFile 3

class CSourceFile : public CObject {
	public:
	CDataFile *theFile;
	CHarvestOptions *itsOptions;
	CErrorLog *itsErrorLog;
	CArray *itsHeaders;
	short itsKind;
	int itsPool;
	

	virtual void ISourceFile(CDataFile *aFile);
	virtual int Compile(void);
	virtual unsigned long GetSourceModDate(void);
	virtual unsigned long GetObjectModDate(void);
	virtual CDataFile *GetObjectFile(void);
	virtual char *GetPathName(char *);
	virtual FILE *StdOpen(char *);
	void Dispose(void);
};

/* CLibraryFile.h */

#include "CSourceFile.h"

class CDataFile;
class CHarvestOptions;

class CLibraryFile : public CSourceFile {

	public:
	virtual void ILibraryFile(CDataFile *aFile);
	virtual int Compile(void);
	virtual unsigned long GetSourceModDate(void);
	virtual CDataFile *GetObjectFile(void);
};

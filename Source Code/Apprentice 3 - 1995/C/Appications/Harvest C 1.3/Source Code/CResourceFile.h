/* CResourceFile.h */

#include "CSourceFile.h"

class CDataFile;
class CHarvestOptions;

class CResourceFile : public CSourceFile {

	public:
	virtual void IResourceFile(CDataFile *aFile);
	virtual int Compile(void);
	virtual unsigned long GetSourceModDate(void);
	virtual CDataFile *GetObjectFile(void);
};

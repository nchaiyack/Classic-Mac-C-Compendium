/* CScrap.c */

#define COMPILING_CSCRAP_C
#include "CScrap.h"
#include "Memory.h"


CScrap*		Scrap;


/* initialize the scrap */
/* */		CScrap::CScrap()
	{
		APRINT(("+CScrap::CScrap"));
		ScrapType = 0;
		ScrapData = NIL;
		ScrapChanged = False;
		BackupScrapType = 0;
		BackupScrapData = NIL;
		BackupScrapChanged = False;
		APRINT(("-CScrap::CScrap"));
	}


/* initialize by importing the desk scrap */
void		CScrap::IScrap()
	{
		APRINT(("+CScrap::IScrap"));
		ImportScrap();
		APRINT(("-CScrap::IScrap"));
	}


/* */		CScrap::~CScrap()
	{
		APRINT(("+CScrap::~CScrap"));
		if (ScrapData != NIL)
			{
				ReleaseHandle(ScrapData);
			}
		if (BackupScrapData != NIL)
			{
				ReleaseHandle(BackupScrapData);
			}
		APRINT(("-CScrap::~CScrap"));
	}


/* move scrap to global clipboard */
void		CScrap::ExportScrap(void)
	{
		APRINT(("+CScrap::ExportScrap"));
		ZeroScrap();
		if (ScrapData != NIL)
			{
				MoveHHi(ScrapData);
				HLock(ScrapData);
				PutScrap(HandleSize(ScrapData),ScrapType,*ScrapData);
				HUnlock(ScrapData);
			}
		ScrapChanged = False;
		APRINT(("-CScrap::ExportScrap"));
	}


/* move global clipboard to private scrap */
void		CScrap::ImportScrap(void)
	{
		long		Offset;

		APRINT(("+CScrap::ImportScrap"));
		if (ScrapData != NIL)
			{
				ReleaseHandle(ScrapData);
			}
		if (BackupScrapData != NIL)
			{
				ReleaseHandle(BackupScrapData);
				BackupScrapData = NIL;
			}
		ScrapData = AllocHandle(0);
		SetTag(ScrapData,"ScrapData");
		GetScrap(ScrapData,'TEXT',&Offset);
		ScrapType = 'TEXT';
		BackupScrapType = 0;
		ScrapChanged = False;
		BackupScrapChanged = False;
		APRINT(("-CScrap::ImportScrap"));
	}


/* return a copy of the private scrap data */
Handle	CScrap::GetScrapData(void)
	{
		Handle		Temp;

		APRINT(("+CScrap::GetScrapData"));
		Temp = AllocHandleCanFail(HandleSize(ScrapData));
		if (Temp == NIL)
			{
				Temp = AllocHandle(0);
				SetTag(Temp,"ScrapCopy");
				APRINT(("-CScrap::GetScrapData"));
				return Temp;  /* failed, so return an empty handle */
			}
		 else
			{
				SetTag(Temp,"ScrapCopy");
				MemCpy(*Temp,*ScrapData,HandleSize(ScrapData));
				APRINT(("-CScrap::GetScrapData"));
				return Temp;
			}
	}


/* return the private scrap type */
OSType	CScrap::GetScrapType(void)
	{
		APRINT(("+CScrap::GetScrapType"));
		return ScrapType;
		APRINT(("-CScrap::GetScrapType"));
	}


/* delete the old scrap and create a new one */
void		CScrap::SetScrap(Handle Scrap, OSType Type)
	{
		APRINT(("+CScrap::SetScrap"));
		if (BackupScrapData != NIL)
			{
				ReleaseHandle(BackupScrapData);
			}
		BackupScrapData = ScrapData;
		BackupScrapType = ScrapType;
		BackupScrapChanged = ScrapChanged;
		ScrapData = Scrap;
		ScrapType = Type;
		ScrapChanged = True;
		APRINT(("-CScrap::SetScrap"));
	}


/* swap the scrap and backup scrap */
void		CScrap::UndoScrapOp(void)
	{
		OSType		TempType;
		Handle		TempData;
		MyBoolean	TempChanged;

		APRINT(("+CScrap::UndoScrapOp"));
		TempType = BackupScrapType;
		TempData = BackupScrapData;
		TempChanged = BackupScrapChanged;
		BackupScrapType = ScrapType;
		BackupScrapData = ScrapData;
		BackupScrapChanged = ScrapChanged;
		ScrapType = TempType;
		ScrapData = TempData;
		ScrapChanged = TempChanged;
		APRINT(("-CScrap::UndoScrapOp"));
	}


/* export scrap if it was changed, and then delete myself */
void		CScrap::CommitSuicide(void)
	{
		APRINT(("+CScrap::CommitSuicide"));
		if (ScrapChanged)
			{
				ExportScrap();
			}
		delete this;
		APRINT(("-CScrap::CommitSuicide"));
	}

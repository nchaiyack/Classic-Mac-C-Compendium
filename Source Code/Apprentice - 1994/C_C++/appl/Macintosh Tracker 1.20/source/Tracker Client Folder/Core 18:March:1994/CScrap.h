/* CScrap.h */

#pragma once

#include "CObject.h"

struct	CScrap	:	CObject
	{
		OSType		ScrapType;
		Handle		ScrapData;
		MyBoolean	ScrapChanged;
		OSType		BackupScrapType;
		Handle		BackupScrapData;
		MyBoolean	BackupScrapChanged;

		/* */		CScrap();
		/* */		~CScrap();

		/* initialize by importing */
		void		IScrap(void);

		/* move scrap to global clipboard */
		void		ExportScrap(void);

		/* move global clipboard to private scrap */
		void		ImportScrap(void);

		/* return a copy of the private scrap data */
		Handle	GetScrapData(void);

		/* return the private scrap type */
		OSType	GetScrapType(void);

		/* delete the old scrap and create a new one */
		void		SetScrap(Handle Scrap, OSType Type);

		/* undo last scrap operation (by swapping the scrap handles) */
		void		UndoScrapOp(void);

		/* export scrap IF IT CHANGED, and then delete yourself */
		void		CommitSuicide(void);
	};

#ifndef COMPILING_CSCRAP_C
	extern CScrap*	Scrap;
#endif

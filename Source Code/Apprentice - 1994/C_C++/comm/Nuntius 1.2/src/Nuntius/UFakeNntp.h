// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UFakeNntp.h

#define __UFAKENNTP__

#ifndef __UNNTP__
#include "UNntp.h"
#endif

class PFakeNntp : public PNntp 
{
	public:
		// NOTE: The caller owns the returned handle
		Handle GetListOfAllGroups();
		Handle GetListOfNewGroups(unsigned long fromDate); // = 0
		
		// NOTE: The caller owns the returned handle
		Handle GetListOfGroupDesciptions();
		
		// NOTE: The caller owns the returned handle
		Handle GetHeaderList(const char *headerName, long firstArticleID, long lastArticleID);
		
		// NOTE: The caller owns the returned handle
		Handle GetArticle(long articleID);
		
		// Note: The caller owns the handle
		virtual void PostArticle(Handle h, short ackStringID);

		Boolean IsPostingAllowed();
		
		void ExamineNewsServer();

		PFakeNntp();
		void IFakeNntp();
		~PFakeNntp();
	protected:
		long fRealFirstArticleID, fRealLastArticleID;
		CStr63 fPath;

		void DoSetGroup(const CStr255 &name);
		Handle ReadFile(const CStr255 &name);
};

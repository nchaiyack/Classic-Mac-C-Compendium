// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// URealNntp.h

#define __UREALNNTP__

#ifndef __UNNTP__
#include "UNntp.h"
#endif

#ifndef __UNETASCIIPROTOCOL__
#include "UNetAsciiProtocol.h"
#endif

class PRealNntp : public PNntp, public PNetAsciiProtocol
{
	public:
		// All retuned handles are null terminated like c strings
		// and they are owned by the caller
		
		Handle GetListOfAllGroups();
		Handle GetListOfNewGroups(unsigned long fromDate);
		
		Handle GetListOfGroupDesciptions();
		// OBS: uses non-standard "list newsgroups" command.
		// If the server returns "unknown command" errors, an empty handle is returned
		
		Handle GetHeaderList(const char *headerName, long firstArticleID, long lastArticleID);
		
		Handle GetArticle(long articleID);
		
		void PostArticle(Handle h, short ackStringID);

		Boolean IsPostingAllowed(); // general, not for the specific group
		
		virtual void ExamineNewsServer();

		PRealNntp();
		void IRealNntp(long newsServerAddr);
		~PRealNntp();
	private:
		Boolean fPostingAllowed;
		Boolean fAvoidListOfGroupDescriptions;
		Ptr fCmdP, fAltCmdP;
		// command buffer (c-string)
		// should not be CR/LF terminated, only text as C-string
		

		void DoFailNntpError(char *cmd);
		void DoSetGroup(const CStr255 &name);
		void DoCmd();
		void DoAuth();
		void DoSingleCommand(char *cmd);
		void SayGoodbye();
};

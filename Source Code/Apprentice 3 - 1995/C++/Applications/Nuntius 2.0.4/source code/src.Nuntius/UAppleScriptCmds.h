// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UAppleScriptCmds.h

#define __UMAILCMDS__

#ifndef __OSA__
#include <OSA.h>
#endif

class TArticle;
class TArticleListView;
class TGroupDoc;

OSErr LoadAppleScript();
void FindScriptInStandardFolders(const CStr255 &appleScriptName, FSSpec &spec, HParamBlockRec &pb);
void ShowNuntiusHomeSite();
void OpenNuntiusWebPage();

class TStandardAppleScript : public TObject
{
DeclareClass(TStandardAppleScript);
public:
	OSAID GetScriptID();
	
	void SaveScript();

	TStandardAppleScript();
	void IStandardAppleScript(const CStr255 &name); // loads script
	virtual void Free();
protected:
	CStr255 fAppleScriptName;
	OSAID fScriptID;
	TFile *fScriptFile;
	Handle fScriptData;
	Boolean fScriptComesFromTextFile;

	void LoadScriptFromTextFile(const FSSpec &spec);
	void LoadScriptFromCompiledFile(const FInfo &fndrInfo, const FSSpec &spec);
	void LoadScriptFromStandardFolders();
};
	
class TSendEMailCommand : public TCommand
{
DeclareClass(TSendEMailCommand);
public:
	void DoIt();
	
	TSendEMailCommand();
	void ISendEMailCommand(const CStr255 &subject, Handle recipientsH, Handle bodyH);
	void Free();
private:
	CStr255 fSubject;
	Handle fTypeH, fRecipientListH, fBodyH;
	TStandardAppleScript *fAppleScript;
	AEDesc fParamListDesc;
	
	void LaunchEudora();
	Boolean ApplicationIsInfront(ProcessSerialNumber &appPSN);
	void PutBodyIntoScrap();
	void CreateTypeText();
	void ExecuteHack();
//
	void ExecuteAppleScript();
	void DoExecuteAppleScript();
};

class TMailToAuthorCommand : public TCommand
{
DeclareClass(TMailToAuthorCommand);
public:
	void DoIt();
	
	TMailToAuthorCommand();
	void IMailToAuthorCommand(TGroupDoc *doc, TArticleListView *articleListView, 
			const CStr255 &subject, Boolean asQuote, Boolean addHeader);
	void Free();
private:
	TArticleListView *fArticleListView;
	TArticle *fArticle;
	Handle fQuoteH;
	Handle fRecipientListH;
	TGroupDoc *fDoc;
	CStr255 fSubject;
	Boolean fAsQuote;
	Boolean fAddHeader;
	TLongintList *fArticleIDList;

	void CreateRecipientList();
};

class TOpenURLCommand : public TCommand
{
DeclareClass(TOpenURLCommand);
public:
	void DoIt();
	
	TOpenURLCommand();
	void IOpenURLCommand(const CStr255 &url);
	void IOpenURLCommand(Handle urlH);
	void Free();
private:
	CStr255 fURL, fScheme;
	Handle fURLH;
	TStandardAppleScript *fAppleScript;
	AEDesc fParamListDesc;

	void ExtractScheme();
	void HandleMailURL();
	void ExecuteScript();
};

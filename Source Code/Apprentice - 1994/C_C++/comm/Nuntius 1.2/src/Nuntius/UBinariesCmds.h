// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UBinariesCmds.h

#define __UBINARIESCMDS__

class TGroupDoc;
class TArticle;
class TNewBinCommand;
class PBinaryDecoder;
class PString255Array;

class TProcessArticleLinesCommand : public TCommand
{
	public:
		virtual pascal void DoIt();

		TProcessArticleLinesCommand();
		virtual pascal void Initialize();
		virtual void IProcessArticleLinesCommand(CommandNumber itsCommandNumber,
													TGroupDoc *doc, TLongintList *idList, // _OWNS_ the list
													Boolean processHeaders);
		virtual pascal void Free();

	protected:
		TLongintList *fIDList;
		PString255Array *fSubjectList;
		CStr255 fGroupDotName;
		TFile *fFile;
		CStr255 fFileName;
		Boolean fProcessHeaders;
		long fNumArticleLines, fLineNo;
		ParamBlockRec *fPBP;
	
		virtual void DoPreProcessing();
		virtual void SetupProgressTexts();
		virtual void ProcessOneArticle(long id, long work1, long work2);
		virtual void SetArticleProgressText(long id); // = 0
		virtual void DoPreProcessArticle(TArticle *article);
		virtual void ProcessArticleLines(TArticle *article, long work1, long work2);
		virtual void ProcessLine(TArticle *article, CStr255 &text);
		virtual void DoPostProcessArticle(TArticle *article);
		virtual void DoPostProcessing();

		virtual void FlushBuffer();
		virtual void WriteBytes(const char *p, long noBytes);
		virtual void WriteString(const CStr255 &txt);
		virtual void WriteLine(const CStr255 &txt);
		virtual void WriteChar(char ch);

		virtual void MakeDefaultFilename(CStr255 &filename);
		virtual void AskFileName();
		virtual void GetAskFileNamePromt(CStr255 &prompt); // = 0
		virtual void OpenFile();
		virtual void CloseFile();

		virtual void GetSubject(long id, CStr255 &s);
	private:
		friend class TNewBinCommand;
		Handle fBufferH;
		long fBytesInBuffer;

		virtual void ProcessAllArticles();
};
//---------------------------------------------------------------------
class TSaveArticlesCommand : public TProcessArticleLinesCommand
{
	public:
		virtual pascal void DoIt();

		TSaveArticlesCommand();
		virtual pascal void Initialize();
		virtual void ISaveArticlesCommand(CommandNumber itsCommandNumber,
													TGroupDoc *doc, TLongintList *idList); // _OWNS_ the list
		virtual pascal void Free();
	protected:
		long fNumMissingArticles;
		
		virtual void GetAskFileNamePromt(CStr255 &prompt);

		virtual void ProcessOneArticle(long id, long work1, long work2);
		virtual void DoPreProcessArticle(TArticle *article);
		virtual void SetupProgressTexts();
		virtual void ProcessLine(TArticle *article, CStr255 &text);
		virtual void DoPostProcessArticle(TArticle *article);
};

//---------------------------------------------------------------------

class TNewBinCommand : public TProcessArticleLinesCommand
{
	public:
		virtual pascal void DoIt();

		TNewBinCommand();
		virtual pascal void Initialize();
		virtual void INewBinCommand(CommandNumber itsCommandNumber,
													TGroupDoc *doc, TLongintList *idList); // _OWNS_ the list
		virtual pascal void Free();

	protected:
		virtual void DoPreProcessing(); // sorts the articles!
		virtual void SetupProgressTexts();
		virtual void ProcessLine(TArticle *article, CStr255 &text);
		virtual void DoPostProcessing(); // launches extractor

		virtual void AskFileName();
		virtual void GetAskFileNamePromt(CStr255 &prompt);
		virtual void OpenFile();
		virtual void CloseFile();
		virtual void FlushBuffer();

	private:
		PBinaryDecoder *fSecretDecoderRing;
		Boolean fExtractorNeeded;
		
		OSErr ScanSubject(long id, short &part, short &noParts);
		void DoTheSorting();
};
//---------------------------------------------------------------------
void ExtractBinaries(CommandNumber itsCommandNumber, TGroupDoc *doc, TLongintList *idList);

// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UBinariesCmds.cp

#include "UBinariesCmds.h"
#include "UBinDecoders.h"
#include "UArticleCache.h"
#include "UArticle.h"
#include "UThread.h"
#include "UProgress.h"
#include "UGroupDoc.h"
#include "Tools.h"
#include "FileTools.h"
#include "UPrefsDatabase.h"
#include "ProcessTools.h"
#include "NetAsciiTools.h"
#include "UDynDynArray.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>

#include <OSUtils.h>
#include <ToolUtils.h>
#include <Folders.h>
#include <Packages.h>
#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyArticle

#define qDebugExtract					qDebug
#define qDebugExtractSorting	qDebug
#define qDebugScanSubject			qDebug

#if qDebug
#define DD(x) fprintf(stderr, x)
#else
#define DD(x)
#endif

#if qDebugExtract
#define DDE(x) fprintf(stderr, x)
#else
#define DDE(x)
#endif


const long kBufferSize = 16 * 1024;
const long kProgressScale = 10000;

TProcessArticleLinesCommand::TProcessArticleLinesCommand()
{
}

pascal void TProcessArticleLinesCommand::Initialize()
{
	inherited::Initialize();
	fIDList = nil;
	fSubjectList = nil;
	fGroupDotName = "";
	fFile = nil;
	fBufferH = nil;
	fBytesInBuffer = 0;
	fFileName = "";
	fPBP = nil;
}

void TProcessArticleLinesCommand::IProcessArticleLinesCommand(CommandNumber itsCommandNumber,
													TGroupDoc *doc, TLongintList *idList,
													Boolean processHeaders)
{
	inherited::ICommand(itsCommandNumber, nil, false, false, nil);
	CStr255 s;
	doc->GetGroupDotName(s);
	fGroupDotName = s;
	FailInfo fi;
	if (fi.Try())
	{
		fIDList = idList;
#if qDebug
		if (!IsObject(fIDList))
			ProgramBreak("fIDList is not object!");
#endif	
		fSubjectList = NewString255Array();
		for (ArrayIndex index = 1; index <= fIDList->fSize; ++index)
		{
			HandleOffsetLength hol;
			CStr255 s;
			if (doc->GetSubject(fIDList->At(index), hol))
				CopyHolToCStr255(hol, s);
			else
				s = "?";
			fSubjectList->Append(s);
		}
		fProcessHeaders = processHeaders;
		fBufferH = NewPermHandle(kBufferSize);
		fPBP = (ParamBlockRec*)NewPermPtr(sizeof(ParamBlockRec));		
		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
}

pascal void TProcessArticleLinesCommand::Free()
{
	fBufferH = DisposeIfHandle(fBufferH);
	FreeIfObject(fFile); fFile = nil;
	FreeIfObject(fIDList); fIDList = nil;
	DisposeIfPtr(Ptr(fPBP)); fPBP = nil;
	delete fSubjectList; fSubjectList = nil;
	inherited::Free();
}

//---------------------------------- OUTPUT FILE --------------------------------------
void TProcessArticleLinesCommand::OpenFile()
{
	if (FileExist(fFile))
		FailOSErr(fFile->DeleteFile()); // zap it
	FailOSErr(fFile->CreateFile());
	FailOSErr(fFile->OpenFile());
}

void TProcessArticleLinesCommand::CloseFile()
{
	fFile->CloseFile();
}

//---------------------------------- OUTPUT FILE BUFFER --------------------------------------
void TProcessArticleLinesCommand::FlushBuffer()
{
	HLock(fBufferH);
	WriteASyncToFile(*fPBP, fFile, *fBufferH, fBytesInBuffer);
	HUnlock(fBufferH);
	fBytesInBuffer = 0;
}

void TProcessArticleLinesCommand::WriteBytes(const char *p, long noBytes)
{
	while (true)
	{
		long subBytes = Min(noBytes, kBufferSize - fBytesInBuffer);
		BytesMove(p, *fBufferH + fBytesInBuffer, subBytes);
		fBytesInBuffer += subBytes;
		p += subBytes;
		noBytes -= subBytes;
		if (!noBytes)
			break;
		FlushBuffer();
	}
}

void TProcessArticleLinesCommand::WriteChar(char ch)
{
	WriteBytes(&ch, 1);
}

void TProcessArticleLinesCommand::WriteString(const CStr255 &txt)
{
	if (txt.Length())
	{
		const unsigned char *p = (const unsigned char*)&txt;
		WriteBytes((const char*)p + 1, txt.Length());
	}
}

void TProcessArticleLinesCommand::WriteLine(const CStr255 &text)
{
	WriteString(text);
	WriteChar(13);
}

//---------------------------------- ARTICLE PROCESSING --------------------------------------
pascal void TProcessArticleLinesCommand::DoIt()
{
	if (fIDList->GetSize() < 1)
	{
#if qDebug
		fprintf(stderr, "TProcessArticleLinesCommand::DoIt, did not get any articles to save!!\n");
#endif
		return; // nothing to save
	}
	FailInfo fi;
	if (fi.Try())
	{
		OSType signature;
		if (gPrefs->PrefExists('EDsi'))
			signature = gPrefs->GetSignaturePrefs('EDsi');
		else
			signature = 'ttxt'; // TeachText
		fFile = NewFile('TEXT', signature, 
										kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		AskFileName();
		fFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);

		SetupProgressTexts();
		gCurProgress->SetStandardProgressType();
		gCurProgress->SetWorkToDo(2 * fIDList->GetSize() * kProgressScale);
		gCurProgress->StartProgress(true);

		OpenFile();
		DoPreProcessing();
		ProcessAllArticles();
		FlushBuffer();
		DoPostProcessing();
		CloseFile();
		gCurProgress->WorkDone();
		fi.Success();
	}
	else // fail
	{
		if (fFile)
			fFile->DeleteFile();
		gCurProgress->WorkDone();
		fi.ReSignal();
	}
}

void TProcessArticleLinesCommand::SetupProgressTexts()
{
	SubClassResponsibility();
}

void TProcessArticleLinesCommand::MakeDefaultFilename(CStr255 &filename)
{
	fSubjectList->GetStringAt(1, filename);
	CStr255 part;
	MyGetIndString(part, kSubjectPartHeader);
	short pos = filename.Pos(part);
	if (pos > 1 && pos < filename.Length())
		filename.Delete(pos, filename.Length() - pos + 1);
	CheckFilenameSanity(filename);
}

void TProcessArticleLinesCommand::AskFileName()
{
	CStr255 defaultFilename, prompt;
	MakeDefaultFilename(defaultFilename);
	GetAskFileNamePromt(prompt);
	FSSpec spec;
	::AskFileName(prompt, defaultFilename, spec);
	fFile->Specify(spec);
	fFileName = spec.name;
}

void TProcessArticleLinesCommand::GetAskFileNamePromt(CStr255 & /* prompt */)
{
	SubClassResponsibility();
}

void TProcessArticleLinesCommand::ProcessAllArticles()
{
	long work = 0;
	for (ArrayIndex index = 1; index <= fIDList->GetSize(); index++)
	{
#if qDebugExtract
		fprintf(stderr, "Processes article %ld, id = %ld\n", index, fIDList->At(index));
#endif
		gCurProgress->SetWorkDone(work);
		ProcessOneArticle(fIDList->At(index), work + kProgressScale, work + 2 * kProgressScale);
		work += 2 * kProgressScale;
	}
}

void TProcessArticleLinesCommand::ProcessOneArticle(long id, long work1, long work2)
{
	TArticle *article = nil;
	VOLATILE(article);
	FailInfo fi;
	if (fi.Try())
	{
		SetArticleProgressText(id);
		article = gArticleCache->GetArticle(fGroupDotName, id);
		gCurProgress->SetWorkDone(work1);
		gCurThread->CheckYield();
		DoPreProcessArticle(article);
		gCurThread->CheckYield();
		ProcessArticleLines(article, work1, work2);
		gCurProgress->SetWorkDone(work2);
		gCurThread->CheckYield();
		DoPostProcessArticle(article);
		gCurThread->CheckYield();
		gArticleCache->ReturnArticle(article); article = nil;
		fi.Success();
	}
	else // fail
	{
		gArticleCache->ReturnArticle(article); article = nil;
		fi.ReSignal();
	}
}

void TProcessArticleLinesCommand::ProcessLine(TArticle * /* article */, CStr255 & /* text */ )
{
}

void TProcessArticleLinesCommand::SetArticleProgressText(long id)
{
	CStr255 s;
	GetSubject(id, s);
	gCurProgress->SetText(s);
}

void TProcessArticleLinesCommand::GetSubject(long id, CStr255 &s)
{
	ArrayIndex index = fIDList->GetEqualItemNo(id);
	if (index != kEmptyIndex)
		fSubjectList->GetStringAt(index, s);
	else
		s = "?";
}

void TProcessArticleLinesCommand::ProcessArticleLines(TArticle *article, long work1, long work2)
{
	if (fNumArticleLines <= 0)
		return;
	ArrayIndex noLines, bodyStartLineNo;
	article->GetLineInfo(noLines, bodyStartLineNo);
	ArrayIndex line = fProcessHeaders ? 1 : bodyStartLineNo;
	while (line <= noLines)
	{
		CStr255 text;
		article->GetLine(line, text);
		ProcessLine(article, text);
		fLineNo++;
		long newWork = MinMax(work1, (fLineNo * kProgressScale) / fNumArticleLines + work1, work2);
		gCurProgress->SetWorkDone(newWork);
		gCurThread->CheckYield();
		line++;
	}
}

void TProcessArticleLinesCommand::DoPreProcessArticle(TArticle *article)
{
	long noLines, bodyStartLineNo;
	article->GetLineInfo(noLines, bodyStartLineNo);
	if (!fProcessHeaders)
		noLines -= bodyStartLineNo;
	fNumArticleLines = noLines;
	fLineNo = 0;
}

void TProcessArticleLinesCommand::DoPostProcessArticle(TArticle * /* article */)
{
}

void TProcessArticleLinesCommand::DoPreProcessing()
{
}

void TProcessArticleLinesCommand::DoPostProcessing()
{
}

//********************************************************************************
TSaveArticlesCommand::TSaveArticlesCommand()
{
}

pascal void TSaveArticlesCommand::Initialize()
{
	inherited::Initialize();
	fNumMissingArticles = 0;
}

void TSaveArticlesCommand::ISaveArticlesCommand(CommandNumber itsCommandNumber,
													TGroupDoc *doc, TLongintList *idList)
{
//	Boolean processHeaders = gPrefs->GetBooleanPrefs('Head');
	Boolean processHeaders = true;
	inherited::IProcessArticleLinesCommand(itsCommandNumber, doc, idList, processHeaders);
}

pascal void TSaveArticlesCommand::Free()
{
	inherited::Free();
}

pascal void TSaveArticlesCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		inherited::DoIt();
		fi.Success();
	}
	else // fail
	{
		FailNewMessage(fi.error, fi.message, messageCannotSaveFiles);
	}
}

void TSaveArticlesCommand::SetupProgressTexts()
{
	gCurProgress->SetTitle(kSaveProgressTitle);
	gCurProgress->SetText("");
}

void TSaveArticlesCommand::GetAskFileNamePromt(CStr255 &prompt)
{
	MyGetIndString(prompt, kSaveArticlesPrompt);
}

void TSaveArticlesCommand::ProcessOneArticle(long id, long work1, long work2)
{
	FailInfo fi;
	if (fi.Try())
	{
		inherited::ProcessOneArticle(id, work1, work2);
		fi.Success();
	}
	else // fail
	{
		if (fi.error == errNoSuchArticle)
			fNumMissingArticles++;
		else
			fi.ReSignal();
	}
}

void TSaveArticlesCommand::ProcessLine(TArticle *article, CStr255 &text)
{
	if (article->ContainsJapaneseEncoding() == false)
		TranslateCStr255(text, gNetAscii2Mac);
	WriteLine(text);
}

void TSaveArticlesCommand::DoPreProcessArticle(TArticle *article)
{
	inherited::DoPreProcessArticle(article);
	if (fProcessHeaders)
		return;
	CStr255 text, s, email;

// subject
	article->GetHeader("Subject", text);
	MyGetIndString(s, kPreSubject);
	text.Insert(s, 1);
	MyGetIndString(s, kPostSubject);
	text += s;
	WriteLine(text);

	article->GetHeader("From", text);
	Boolean hadRealName = GetPrintableAuthorName(text, s, email);
	MyGetIndString(text, kPreFrom);
	text += s;
	if (hadRealName)
	{
		MyGetIndString(s, kArticleReal2EmailSeparator);
		text += s;
		text += email;
	}
	MyGetIndString(s, kPostFrom);
	text += s;
	WriteLine(text);

// Date
	MyGetIndString(text, kPreDate);
	article->GetHeader("Date", s);
	text += s;
	MyGetIndString(s, kPostDate);
	text += s;
	WriteLine(text);

// separator
	WriteLine("");
}

void TSaveArticlesCommand::DoPostProcessArticle(TArticle *article)
{
	if (fNumMissingArticles)
	{
		if (fIDList->GetSize() == 1)
			FailOSErr(errNoSuchArticle);
		if (fNumMissingArticles > 1)
		{
			CStr255 s;
			NumToString(fNumMissingArticles, s);
			ParamText(s, gEmptyString, gEmptyString, gEmptyString);
			StdAlert(phMissingSaveArticleMany);
		}
		else
			StdAlert(phMissingSaveArticle);
	}
	CStr255 text;
	MyGetIndString(text, kSaveArticleSeparator);
	WriteLine(text);
	inherited::DoPostProcessArticle(article);
}

/////////////////////////////////////////////////////////////////////////////////
TNewBinCommand::TNewBinCommand()
{
}

pascal void TNewBinCommand::Initialize()
{
	inherited::Initialize();
	fSecretDecoderRing = nil;
	fExtractorNeeded = false;
}

void TNewBinCommand::INewBinCommand(CommandNumber itsCommandNumber,
													TGroupDoc *doc, TLongintList *idList)
{
	inherited::IProcessArticleLinesCommand(itsCommandNumber, doc, idList, false);
}

pascal void TNewBinCommand::Free()
{
	FreeIfPtrObject(fSecretDecoderRing); fSecretDecoderRing = nil;
	inherited::Free();
}

pascal void TNewBinCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		inherited::DoIt();
		fi.Success();
	}
	else // fail
	{
		if (fSecretDecoderRing)
			fSecretDecoderRing->Abort();
		FailNewMessage(fi.error, fi.message, messageCannotExtractBinaries);
	}
}

void TNewBinCommand::AskFileName()
{
}

void TNewBinCommand::GetAskFileNamePromt(CStr255 &prompt)
{
	MyGetIndString(prompt, kSaveBinariesPrompt);
}

void TNewBinCommand::ProcessLine(TArticle * /* article */, CStr255 &text)
{
	CStr255 s;
	if (fSecretDecoderRing)
	{
		fSecretDecoderRing->DecodeLine(text);
		return;
	}
	if (!text.Length())
		return;
	unsigned char len = text.Length(); // easier to read
	char *p = (char*)&text[1];
	if (strncmp(p, "(This file must be converted with BinHex 4.0", 40) == 0)
	{
		DDE("Found '(This file mustÉ' -> kBinHexWaitForStartColon\n");
		PBinHexDecoder *bhd = new PBinHexDecoder();
		bhd->IBinHexDecoder(fFile);
		fSecretDecoderRing = bhd;
		fExtractorNeeded = true;
		fSecretDecoderRing->DecodeLine(text);
	}
	else if (strncmp(p, "begin ", 6) == 0 && len < 100)
	{
		text += 32; // space to ease parsing
		char *cp = p + 6;
		while (*cp > 32)
		{
			if (*cp < '0' || *cp >= '8')
			{
				DD("Found 'begin ', but number was not octal, line is ignored\n");
				return; // not octal
			}
			++cp;
		}
		long mode;
		char buffer[200];
		if (sscanf(p, "begin %ld %s", &mode, &buffer) != 2)
		{
			DD("Could not sscanf the uuencode filename, line is ignored\n");
			return;
		}
//		if (1 || gPrefs->GetBooleanPrefs('UUna'))
//		{
			s = buffer;
			CheckFilenameSanity(s);
			FSSpec spec;
			gPrefs->GetSilentDirAliasPrefs('FBin', spec);
			CopyCString2String(s, spec.name);
			MakeFilenameUnique(spec);
			s = spec.name;
			if (gPrefs->GetBooleanPrefs('BiAs'))
			{
				CStr255 prompt;
				GetAskFileNamePromt(prompt);
				::AskFileName(prompt, s, spec);
			}
//		}
		fFileName = s;
		fFile->Specify(spec);
		FailOSErr(fFile->CreateDataFork());
		PUUDecoder *uud = new PUUDecoder();
		uud->IUUDecoder(fFile);
		fSecretDecoderRing = uud;
		uud->PrepareUU();
		s = fFileName;
		if (s.Length() >= 5)
			s.Delete(1, s.Length() - 4);
		UprString(s, false);
		if (s == ".GIF")
		{
			DDE("File was detected as GIF -> changing file type & creator\n");
			ChangeFileTypeAndCreator(fFile, 'GCon', 'GIFf');
		}
		else if (s == ".JPG")
		{
			DDE("File was detected as JPEG -> changing file type & creator\n");
			ChangeFileTypeAndCreator(fFile, 'JVWR', 'JFIF');
		}
		else
		{
#if qDebugExtract
			fprintf(stderr, "Unknown file format: file saved as 'TEXT'\n");
			fprintf(stderr, "- filename = '%s'\n", (char*)fFileName);
#endif
		}
	}
}

OSErr TNewBinCommand::ScanSubject(long id, short &part, short &noParts)
{
	// just _why_ are no one using a "Part x of y" header line???
	CStr255 s;
	GetSubject(id, s);
#if qDebugScanSubject
	fprintf(stderr, "ScanSubject of '%s'\n", (char*)s);
	fprintf(stderr, "                ");
	for (short i = 1; i <= s.Length(); ++i)
		fprintf(stderr, "%ld", long(i)%10);
	fprintf(stderr, "\n");
#endif
	if (s.Length() < 3)
	{
#if qDebugScanSubject
	fprintf(stderr, "  too short -> errBinHexBadSubject\n");
#endif
		return errBinHexBadSubject;
	}
	if ( (s[1] == 'R' || s[1] == 'r')
	&&   (s[2] == 'e' || s[2] == 'E')
	&&   (s[3] == ':'))
	{
#if qDebugScanSubject
				fprintf(stderr, "  starts with 'Re:' -> errBinHexBadSubject\n");
#endif
				return errBinHexBadSubject;
	}
	short index = s.Length();
TryWithNextNumber:
	while (index && (s[index] < '0' || s[index] > '9')) 
		--index;
	if (index == 0)
	{
#if qDebugScanSubject
		fprintf(stderr, "  couldn't find number -> errBinHexBadSubject\n");
#endif
		return errBinHexBadSubject;
	}
#if qDebugScanSubject
	fprintf(stderr, "  found number end-at %ld\n", long(index));
#endif
	short faktor = 1;
	noParts = 0;
	while (index && s[index] >= '0' && s[index] <= '9')
	{
		noParts += faktor * (s[index] - '0');
		faktor *= 10;
		--index;
	}
#if qDebugScanSubject
	fprintf(stderr, "  got number == noParts == %ld, prev-index = %ld\n", long(noParts), long(index));
#endif
	while (index && s[index] <= 32)
		--index;
#if qDebugScanSubject
	fprintf(stderr, "  prev spaces, index = %ld\n", long(index));
#endif
	if (s[index - 1] == 'o' && s[index] == 'f')
		index -= 2;
	else switch (s[index])
	{
		case '/':
		case '|':
		case ':':
		case '-':
			--index;  // good separator
			break;
		
		case '*':
		case 'x':
		case 'X':		// not part stuff, but resolution etc
#if qDebugScanSubject
			fprintf(stderr, "  got resolution char at index = %ld !> try again\n", long(index));
#endif
			goto TryWithNextNumber;
	
		default:
#if qDebugScanSubject
			fprintf(stderr, "  bad separator at index %ld !> try again\n", long(index));
#endif
			goto TryWithNextNumber; // bad char, cannot be part stuff
	}
#if qDebugScanSubject
	fprintf(stderr, "  prev separator, index = %ld\n", long(index));
#endif
	while (index && s[index] <= 32)
		--index;
#if qDebugScanSubject
	fprintf(stderr, "  prev spaces, index = %ld\n", long(index));
#endif
	if (index <= 0)
	{
#if qDebugScanSubject
		fprintf(stderr, "  no more chars left in string -> errBinHexBadSubject\n");
#endif
		return errBinHexBadSubject;
	}
	if (s[index] < '0' || s[index] > '9')
	{
#if qDebugScanSubject
		fprintf(stderr, "  char at %ld is not digit !> try again\n", long(index));
#endif
		goto TryWithNextNumber;
	}
	faktor = 1;
	part = 0;
	while (index && s[index] >= '0' && s[index] <= '9')
	{
		part += faktor * (s[index] - '0');
		faktor *= 10;
		--index;
	}
#if qDebugScanSubject
	fprintf(stderr, "  got number == part-num == %ld, prev-index = %ld -> OK\n", long(noParts), long(index));
#endif
	return noErr;
}

void TNewBinCommand::DoTheSorting()
{
	TSortedLongintList *partList = nil;
	VOLATILE(partList);
	TLongintList *sortedIDList = nil;
	VOLATILE(sortedIDList);
	FailInfo fi;
	if (fi.Try())
	{
		TSortedLongintList *sllist = new TSortedLongintList();
		sllist->ISortedLongintList();
		partList = sllist;
		
		TLongintList *llist = new TLongintList();
		llist->ILongintList();
		sortedIDList = llist;
		
		// extract the 'part x of y', and check y for consistency
		short noParts = 0;
		short i;
		for (i = 1; i <= fIDList->GetSize(); i++)
		{
			long id = fIDList->At(i);
			short thisPart, thisNoParts;
			OSErr err = ScanSubject(id, thisPart, thisNoParts);
#if qDebugExtractSorting
			fprintf(stderr, "Scan: err = %ld, thisPart = %ld, thisNoParts = %ld\n", long(err), long(thisPart), long(thisNoParts));
#endif
			if (err)
				continue;
			if (thisPart == 0 && thisNoParts)
				continue; // skip
			if (!noParts)
			{
				noParts = thisNoParts; // first one
				if (noParts < 1 || noParts > 100) // no one sends a +100 part file (until MIME!)
					FailOSErr(errBinHexMultiSubjectError);
				if (noParts > fIDList->GetSize())
					FailOSErr(errBinariesMissingPart);
			}
			if (	thisPart < 1 || 
						thisNoParts != noParts ||
						thisPart > noParts)
						// minimum sanity checking!
			{
				FailOSErr(errBinHexMultiSubjectError);
			}
			if (partList->GetEqualItemNo(thisPart))
				FailOSErr(errBinHexMultiSubjectError); // have it already!

			partList->Insert(thisPart);
			while (thisPart > sortedIDList->GetSize())
				sortedIDList->InsertLast(0); // ugly
			sortedIDList->AtPut(thisPart, id);
		}

		// check for missing parts (simple!)
		if (partList->GetSize() < 1) // ups
			FailOSErr(errBinHexMultiSubjectError);
		if (partList->GetSize() < noParts)
			FailOSErr(errBinariesMissingPart);
		if (partList->GetSize() > noParts) // ups
			FailOSErr(errBinHexMultiSubjectError);
		
		// check for bad item-numbers
		for (i = 1; i <= partList->GetSize(); i++)
			if (partList->At(i) != i)
				FailOSErr(errBinHexMultiSubjectError);
		
		// it passed all the tests, so
		// substitute the old id list with the new
		partList->Free();
		fIDList->Free();
		fIDList = sortedIDList;
		sortedIDList = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(partList); partList = nil;
		FreeIfObject(sortedIDList); sortedIDList = nil;
		fi.ReSignal();
	}
}

void TNewBinCommand::DoPreProcessing()
{
	if (fIDList->GetSize() == 1)
		return; // not processing when only one file
		// missing check for only having part x/y out of a serie!
	FailInfo fi;
	if (fi.Try())
	{
#if qDebugExtractSorting
		fprintf(stderr, "Going to sort %ld articles: ", fIDList->GetSize());
		short i;
		for (i = 1; i <= fIDList->GetSize(); i++)
			fprintf(stderr, "%ld, ", fIDList->At(i));
		fprintf(stderr, "\n");
#endif
		DoTheSorting();
#if qDebugExtractSorting
		fprintf(stderr, "Result of sorting %ld articles: ", fIDList->GetSize());
		for (i = 1; i <= fIDList->GetSize(); i++)
			fprintf(stderr, "%ld, ", fIDList->At(i));
		fprintf(stderr, "\n");
#endif
		fi.Success();
	}
	else
		fi.ReSignal();
}

void TNewBinCommand::SetupProgressTexts()
{
	gCurProgress->SetTitle(kExtractProgressTitle);
	gCurProgress->SetText("");
}


void TNewBinCommand::DoPostProcessing()
{
	if (fSecretDecoderRing)
		fSecretDecoderRing->PostProcess();
#if 1
	if (!fExtractorNeeded)
		return;
	if (!gPrefs->GetBooleanPrefs('BLau'))
		return;
	if (!gPrefs->PrefExists('BLid'))
		FailOSErr(errNoExtractor);
	AliasHandle aliasH = nil;
	VOLATILE(aliasH);
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		gPrefs->GetAliasPrefs('BLid', spec);
		if (gPrefs->GetBooleanPrefs('BLOP'))
		{
			fFile->GetAlias(aliasH);
			OpenApplicationDocument(spec, aliasH, false);
		}
		else
			LaunchApplication(spec, false);
		fi.Success();
	}
	else // fail
	{
		DisposeIfHandle(Handle(aliasH));
		FailNewMessage(fi.error, fi.message, messageLaunchExtractorFailed);
	}
#endif
}

void TNewBinCommand::OpenFile()
{
	// the decoder handles this
}

void TNewBinCommand::CloseFile()
{
}

void TNewBinCommand::FlushBuffer()
{
#if qDebug
	if (fBytesInBuffer)
		ProgramBreak("Some people placed bytes in this buffer which is out of use for today");
#endif
}
//********************************************************************************

void ExtractBinaries(CommandNumber itsCommandNumber, TGroupDoc *doc, TLongintList *idList)
{
	TNewBinCommand *aCommand = new TNewBinCommand();
	aCommand->INewBinCommand(itsCommandNumber, doc, idList);
	gApplWideThreads->ExecuteCommand(aCommand, "Extract of binaries");
}

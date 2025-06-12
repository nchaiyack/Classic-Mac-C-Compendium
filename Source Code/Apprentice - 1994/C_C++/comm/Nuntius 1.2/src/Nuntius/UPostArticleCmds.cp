// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPostArticleCmds.cp

#include "UPostArticleCmds.h"
#include "UArticle.h"
#include "UArticleCache.h"
#include "UGroupDoc.h"
#include "UDiscList.h"
#include "UPrefsDatabase.h"
#include "FileTools.h"
#include "Tools.h"
#include "UNntp.h"
#include "UMacTCP.h"
#include "UProgress.h"
#include "ProcessTools.h"
#include "NetAsciiTools.h"
#include "ISO2022Conversion.h"
#include "UBufferedFileStream.h"
#include "UThread.h"
#include "UFatalError.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>

#include <Packages.h>
#include <Errors.h>
#include <UDialog.h>
#include <Folders.h>
#include <Resources.h>
#include <ToolUtils.h>
#include <OSUtils.h>
#include <Script.h>

#pragma segment MyArticle

#define qDebugGarbageCollector qDebug & 0
//==================================================================================================
PPostArticleInfo::PPostArticleInfo()
{
	fSubject = "";
	fNewsGroups = "";
	fReferences = "";
	fDistribution = "";
	fFile = nil;
	fDefaultTextH = nil;
	fRefCount = 0;
	fEditHeaders = false;
	fEditSignature = false;
	fAddSignatureChoice = false;
	fTrashDiskCopyAfterPost = false;
	fNntp = nil;
}

void PPostArticleInfo::IPostArticleInfo()
{
	fEditHeaders = gPrefs->GetBooleanPrefs('EdHe');
	fEditSignature = gPrefs->GetBooleanPrefs('EdSi');
	fAddSignatureChoice = gPrefs->GetBooleanPrefs('DUSi');
	fDefaultTextH = NewPermHandle(0);
}

PPostArticleInfo::~PPostArticleInfo()
{
	gNntpCache->ReturnNntp(fNntp); fNntp = nil;
	FreeIfObject(fFile); fFile = nil;
	fDefaultTextH = DisposeIfHandle(fDefaultTextH);
}

void PPostArticleInfo::Cancel()
{
	fCreateArticleW->CloseByUser();
	fCreateArticleW = nil;
	fPostArticleW->CloseByUser();
	fPostArticleW = nil;
	delete this;
}

void PPostArticleInfo::CreateWindows()
{
	fCreateArticleW = gViewServer->NewTemplateWindow(kEditArticleToPostView, nil);
#if qDebug
	if (!IsObject(fCreateArticleW))
		ProgramBreak("fCreateArticleW is nil");
#endif
	fCreateArticleView = (TEditArticleToPostDialogView*) (fCreateArticleW->FindSubView('DLOG'));
#if qDebug
	if (!IsObject(fCreateArticleView))
		ProgramBreak("fCreateArticleView not found");
#endif
	fCreateArticleView->SetInfo(this);
	fCreateArticleW->Open();
//
	fPostArticleW = gViewServer->NewTemplateWindow(kPostArticleView, nil);
#if qDebug
	if (!IsObject(fPostArticleW))
		ProgramBreak("fPostArticleW not created");
#endif
	fPostArticleView = (TPostArticleDialogView*) (fPostArticleW->FindSubView('DLOG'));
#if qDebug
	if (!IsObject(fPostArticleView))
		ProgramBreak("fPostArticleView not found");
#endif
	fPostArticleView->SetInfo(this);
}

void PPostArticleInfo::ShowPostArticleWindow()
{
	fCreateArticleW->Close();
	fPostArticleView->SetInfo(this); // setup Subject string
	fPostArticleW->Select();
	fPostArticleW->Open();
}
//--------------------------------------------------------------------------
void PPostArticleInfo::DoPostArticle()
{
	fCreateArticleW->Close();
	fPostArticleW->Close();
	TPostArticleCommand *command = new TPostArticleCommand();
	command->IPostArticleCommand(this);
	gApplWideThreads->ExecuteCommand(command, "TPostArticleCommand");
}

void PPostArticleInfo::CreateNntp()
{
	if (fNntp)
		return;
	fNntp = gNntpCache->GetNntp();
	if (!fNntp->IsPostingAllowed())
		FailOSErr(errNotAllowedToPost);
}

void PPostArticleInfo::DiscardNntp()
{
	gNntpCache->DiscardNntp(fNntp); fNntp = nil;
}

//--------------------------------------------------------------------------
void PPostArticleInfo::MakeFile()
{
	TStream *aStream = nil;
	VOLATILE(aStream);
	TFile *file = nil;
	VOLATILE(file);
	FailInfo fi;
	if (fi.Try())
	{
		OSType signature;
		if (gPrefs->PrefExists('EDsi'))
			signature = gPrefs->GetSignaturePrefs('EDsi');
		else
			signature = 'ttxt'; // TeachText
		file = NewFile('TEXT', signature,
			kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		file->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		FSSpec spec;
		gPrefs->GetSilentDirAliasPrefs('FEdi', spec);
		CStr255 name(fSubject);
		CheckFilenameSanity(name);
		CopyCString2String(name, spec.name);
		file->Specify(spec);
		MakeFilenameUnique(file);
		FailOSErr(file->CreateFile());
		FailOSErr(file->OpenFile());
		aStream = NewBufferedFileStream(file, 0, 16 * 1024);
		
		if (fEditHeaders)
			MakeHeader(aStream, kPostArticleHeaderStrings);
		
		HLock(fDefaultTextH);
		aStream->WriteBytes(*fDefaultTextH, GetHandleSize(fDefaultTextH));
		HUnlock(fDefaultTextH);
		
		if (fEditSignature && fAddSignatureChoice)
			MakeSignature(aStream);
		
		aStream->Free(); aStream = nil;
		FailOSErr(file->CloseFile());
		fFile = file; file = nil;
		fFile->Modified();
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		FreeIfObject(file); file = nil;
		fi.ReSignal();
	}
}

void PPostArticleInfo::LaunchEditor()
{
	FailNIL(fFile);
	AliasHandle aliasH;
	fFile->GetAlias(aliasH);
	VOLATILE(aliasH);
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		gPrefs->GetAliasPrefs('EDid', spec);
		OpenApplicationDocument(spec, aliasH, true);
		DisposeIfHandle(Handle(aliasH));
		fi.Success();
	}
	else // fail
	{
		DisposeIfHandle(Handle(aliasH));
		if (fi.error)
			gApplication->ShowError(fi.error, messageLaunchEditorFailed);
	}
}

//==========================================================================

void WriteString(TStream *aStream, const CStr255 &text)
{
	CStr255 s(text); //?? until CStr255::operator[] const is inlined
	aStream->WriteBytes(&s[1], text.Length());
}

void Num2TwoDigitString(long l, CStr255 &s)
{
	s.Length() = 2;
	s[1] = char('0' + l / 10);
	s[2] = char('0' + (l % 10));
}

void PPostArticleInfo::MakeHeader(TStream *aStream, short templateID)
{
#if qDebug
	if (!IsObject(aStream))
		ProgramBreak("aStream is not object");
#endif
	CStr255 s, realname, username, email, machinename, dotname;
	CStr255 nuntiusversion, organization, messageID;

	gPrefs->GetStringPrefs('@adr', email);
	username = email;
	short pos = username.Pos("@");
	if (pos)
		username.Delete(pos, username.Length() - pos + 1);

	// bit trouble, as we have to be able to post from an un(dot)named mac
	FailInfo fi;
	if (fi.Try())
	{
		GetMyDotName(dotname);
		fi.Success();
	}
	else // fail
	{
		if (fi.error == errMyMacHasNoDotName || fi.error == noNameServer || fi.error == authNameErr || fi.error == noAnsErr || fi.error == dnrErr)
			dotname = "";
		else
			fi.ReSignal();
	}
	machinename = dotname;
	pos = machinename.Pos(".");
	if (pos)
		machinename.Delete(pos, machinename.Length() - pos + 1);

	UseResFile(gApplicationRefNum); // just to be sure...
	Handle h = GetResource('vers', 1);
	FailNILResource(h);
	HNoPurge(h);
	nuntiusversion = VersRecPtr(*h)->shortVersion;
	gPrefs->GetStringPrefs('Orga', organization);
	gPrefs->GetStringPrefs('Name', realname);
	
	static unsigned long noHeader = 0;
	unsigned long tc = TickCount();
	unsigned long dati;
	GetDateTime(dati);
	long l2 = (MyIP() & 0xFFFF);
	l2 |= (++noHeader & 0xFF) << 16; 
	l2 |= (tc & 0xFF) << 24;
	messageID = '<';
	Long2Hex(dati, s);
	messageID += s;
	Long2Hex(l2, s);
	messageID += s;
	messageID += '@';
	messageID += dotname;
	messageID += '>';

	//    Wdy, DD Mon YY HH:MM:SS TIMEZONE
	DateTimeRec dt;
	Secs2Date(dati, dt);
	CStr255 year, month, day, hour, minute, second, dayofweek, gmthour;
	NumToString(dt.year, year);
	GetIndString(s, kPostArticleMonthNames, dt.month);
	month = s;
	NumToString(dt.day, s);
	day = s;
	Num2TwoDigitString(dt.hour, hour);
	Num2TwoDigitString(dt.minute, minute);
	Num2TwoDigitString(dt.second, second);
	GetIndString(s, kPostArticleWeekDayNames, dt.dayOfWeek);
	dayofweek = s;

	MachineLocation loc;
	ReadLocation(loc);
	long gmt = dt.hour + 24 - short(loc.gmtFlags.gmtDelta & 0xFFFF) / 3600;
	Num2TwoDigitString(gmt % 24, gmthour);

	h = GetResource('STR#', templateID);
	FailNILResource(h);
	short numHeaders = *( (short*) *h);
	for (short index = 1; index <= numHeaders; index++)
	{
		GetIndString(s, templateID, index);
		SubstituteStringItems(s, "ÇrealnameÈ", realname);
		SubstituteStringItems(s, "ÇusernameÈ", username);
		SubstituteStringItems(s, "ÇemailÈ", email);
		if (SubstituteStringItems(s, "ÇmachinenameÈ", machinename) && !machinename.Length())
			continue;
		if (SubstituteStringItems(s, "ÇdotnameÈ", dotname) && !dotname.Length())
			continue;
		SubstituteStringItems(s, "ÇnewsgroupsÈ", fNewsGroups);
		SubstituteStringItems(s, "ÇsubjectÈ", fSubject);
		SubstituteStringItems(s, "ÇnuntiusversionÈ", nuntiusversion);
		if (SubstituteStringItems(s, "ÇreferencesÈ", fReferences) && !fReferences.Length())
			continue;
		if (SubstituteStringItems(s, "ÇdistributionÈ", fDistribution) && !fDistribution.Length())
			continue;
		SubstituteStringItems(s, "ÇorganizationÈ", organization);
		if (SubstituteStringItems(s, "Çmessage-idÈ", messageID) && !dotname.Length())
			continue;
// date
		SubstituteStringItems(s, "ÇyearÈ", year);
		SubstituteStringItems(s, "ÇmonthÈ", month);
		SubstituteStringItems(s, "ÇdayÈ", day);
		SubstituteStringItems(s, "ÇminuteÈ", minute);
		SubstituteStringItems(s, "ÇsecondÈ", second);
		SubstituteStringItems(s, "ÇdayofweekÈ", dayofweek);
		SubstituteStringItems(s, "ÇhourÈ", hour);
		SubstituteStringItems(s, "Çgmt-hourÈ", gmthour);

//
		if (s.Pos("Ç") || s.Pos("È"))
		{
#if qDebug
			fprintf(stderr, "Unknown variable in header: %s\n", (char*)s);
#endif
			continue; // unknown variable, do not emit header
		}
		if (!s.Length() || s[s.Length()] != '\n')
			s += "\n";
		WriteString(aStream, s);
	}
}

void PPostArticleInfo::MakeSignature(TStream *aStream)
{
#if qDebug
	if (!IsObject(aStream))
		ProgramBreak("aStream is not object");
#endif
	if (!gPrefs->PrefExists('Sigu'))
		return; // no signature file
	TFile *sigFile = nil;
	VOLATILE(sigFile);
	Handle dataH = nil;
	VOLATILE(dataH);
	FailInfo fi;
	if (fi.Try())
	{
		sigFile = NewFile('TEXT', 'ttxt',
			kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		sigFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		FSSpec spec;
		gPrefs->GetAliasPrefs('Sigu', spec);
		sigFile->Specify(spec);		
		FailOSErr(sigFile->OpenFile());
		long size;
		FailOSErr(sigFile->GetDataLength(size));
		dataH = NewPermHandle(size);
		HLock(dataH);
		FailOSErr(sigFile->ReadData(*dataH, size));
		aStream->WriteBytes(*dataH, size);
		HUnlock(dataH);
		dataH = DisposeIfHandle(dataH);
		sigFile->CloseFile();
		sigFile->Free(); sigFile = nil;
		fi.Success();
	}
	else // fail
	{
		dataH = DisposeIfHandle(dataH);
		sigFile->Free(); sigFile = nil;
		FailNewMessage(fi.error, fi.message, messageSignatureProblem);
	}
}
//---------------------------------------------------------------------
TCreateNewDiscussionCommand::TCreateNewDiscussionCommand()
{
}


pascal void TCreateNewDiscussionCommand::Initialize()
{
	inherited::Initialize();
}

void TCreateNewDiscussionCommand::ICreateNewDiscussionCommand(TGroupDoc *doc)
{
#if qDebug
	if (!IsObject(doc))
		ProgramBreak("doc is not object");
#endif
	inherited::ICommand(cPostNewDiscussion, nil, false, false, nil);
	PPostArticleInfo *info = new PPostArticleInfo();
	info->IPostArticleInfo();
	fPostArticleInfo = info;
	CStr255 s;
	fDoc = doc;
	fDoc->GetGroupDotName(s);
	fPostArticleInfo->fNewsGroups = s;
}

pascal void TCreateNewDiscussionCommand::Free()
{
	inherited::Free();
}

pascal void TCreateNewDiscussionCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		if (!gPrefs->GetBooleanPrefs('NaOK'))
			FailOSErr(errMissingYourName);
		if (!gPrefs->PrefExists('EDid'))
			FailOSErr(errNoEditor);
		fPostArticleInfo->CreateNntp();
		PrepareInfo();
		fPostArticleInfo->CreateWindows();
		fi.Success();
	}
	else // fail
	{
		delete fPostArticleInfo; fPostArticleInfo = nil;
		FailNewMessage(fi.error, fi.message, messageCouldNotPost);
	}
}

void TCreateNewDiscussionCommand::PrepareInfo()
{
}

//.............................................................

TCreateFollowupCommand::TCreateFollowupCommand()
{
}


pascal void TCreateFollowupCommand::Initialize()
{
	inherited::Initialize();
	fDefaultText = nil;
	fArticle = nil;
}

void TCreateFollowupCommand::ICreateFollowupCommand(TGroupDoc *doc, ArrayIndex discIndex,
						Handle defaultText)
{
#if qDebug
	if (!IsObject(doc))
		ProgramBreak("doc is not object");
#endif
	inherited::ICreateNewDiscussionCommand(doc);
	fDefaultText = defaultText;
	fDiscIndex = discIndex;
	fDoc = doc;
}	

pascal void TCreateFollowupCommand::Free()
{
	gArticleCache->ReturnArticle(fArticle); fArticle = nil;
	fDefaultText = DisposeIfHandle(fDefaultText);
	inherited::Free();
}

void TCreateFollowupCommand::PrepareInfo()
{
	inherited::PrepareInfo();
	PDiscList *discList = fDoc->GetDiscList();
	TLongintList *idList = nil;
	FailInfo fi;
	if (fi.Try())
	{
		idList = discList->GetArticleIDList(fDiscIndex);
		DoPrepareArticleInfo(idList->At(1));
		long noID = idList->GetSize();
		long noQuoted = MinMax(0, noID - 2, 4);
		for (long i = 2; i <= noQuoted; i++)
		{
			CStr255 s;
			long index = noID - noQuoted + i;
			long id = idList->At(index);
			if (id < fDoc->GetFirstArticleID() || id > fDoc->GetLastArticleID())
				continue;
			if (!GetMsgID(id, s))
				continue;
			if (fPostArticleInfo->fReferences.Length() + s.Length() > 150)
				continue;
			fPostArticleInfo->fReferences += ' ';
			fPostArticleInfo->fReferences += s;
		}
		FreeIfObject(idList); idList = nil;
#if qDebug & 0
		CStr255 zzz(fPostArticleInfo->fReferences);
		fprintf(stderr, "References for follow-up: '%s'\n", (char*)zzz);
#endif

		DisposeIfHandle(fPostArticleInfo->fDefaultTextH);
		fPostArticleInfo->fDefaultTextH = fDefaultText; fDefaultText = nil;

		fi.Success();
	}
	else // fail
	{
		FreeIfObject(idList); idList = nil;
		fi.ReSignal();
	}
}

void TCreateFollowupCommand::DoPrepareArticleInfo(long id)
{
	CStr255 groupDotName;
	fDoc->GetGroupDotName(groupDotName);
	fArticle = gArticleCache->GetArticle(groupDotName, id);
	CStr255 s;
// Subject
	fArticle->GetHeader("Subject", s);
	if (s.Copy(1, 4) != "Re: ")
		if (s.Copy(1, 4) != "re: ")
			s.Insert("Re: ", 1);
	fPostArticleInfo->fSubject = s;
// Message-id
	fArticle->GetHeader("Message-id", s);
	fPostArticleInfo->fReferences = s;
// Newsgroups
	if (fArticle->GetHeader("Newsgroups", s))
		fPostArticleInfo->fNewsGroups = s;
// Distribution
	if (fArticle->GetHeader("Distribution", s))
		fPostArticleInfo->fDistribution = s;
//-
	gArticleCache->ReturnArticle(fArticle); fArticle = nil;
}

Boolean TCreateFollowupCommand::GetMsgID(long articleID, CStr255 &id)
{
	TArticle *article = nil;
	VOLATILE(article);
	FailInfo fi;
	if (fi.Try())
	{
		CStr255 groupDotName;
		fDoc->GetGroupDotName(groupDotName);
		article = gArticleCache->GetArticle(groupDotName, articleID);
		article->GetHeader("Message-id", id);
		gArticleCache->ReturnArticle(article); article = nil;
		fi.Success();
		return true;
	}
	else // fail
	{
		gArticleCache->ReturnArticle(article); article = nil;
		if (fi.error != errNoSuchArticle)
			fi.ReSignal();
		return false;
	}
}

//---------------------------------------------------------------------

TPostArticleCommand::TPostArticleCommand()
{
}


pascal void TPostArticleCommand::Initialize()
{
	inherited::Initialize();
	fText = nil;
	fPostArticleInfo = nil;
}

void TPostArticleCommand::IPostArticleCommand(PPostArticleInfo *info)
{
	inherited::ICommand(cPostArticle, nil, false, false, nil);
	fPostArticleInfo = info;
}

pascal void TPostArticleCommand::Free()
{
	if (fPostArticleInfo)
	{
		delete fPostArticleInfo;
		fPostArticleInfo = nil;
	}
	DisposeIfHandle(fText); fText = nil;
	inherited::Free();
}

pascal void TPostArticleCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		if (fPostArticleInfo->fFile && !fPostArticleInfo->fFile->IsModified())
			FailOSErr(errArticleNotEdited);

		gCurProgress->SetStandardProgressType();
		gCurProgress->SetTitle(CStr255(fPostArticleInfo->fSubject));
		gCurProgress->SetText(kPostArticleExaminesArticle);
		gCurProgress->StartProgress(true);
		gCurProgress->SetWorkToDo(kCandyStribes);

		ReadArticle();
		PostArticle();
		gCurProgress->WorkDone();
		if (fPostArticleInfo->fTrashDiskCopyAfterPost)
			TrashDiskCopy();
		fi.Success();
	}
	else // fail
	{
		gCurProgress->WorkDone();
		FailNewMessage(fi.error, fi.message, messageCouldNotPostArticle);
	}
}

void TPostArticleCommand::ReadArticle()
{
	THandleStream *handleStream = nil;
	VOLATILE(handleStream);
	FailInfo fi;
	if (fi.Try())
	{
// header
		fText = NewPermHandle(0);
		if (!fPostArticleInfo->fEditHeaders)
		{
			THandleStream *hs = new THandleStream();
			hs->IHandleStream(fText, 128);
			handleStream = hs;
			fPostArticleInfo->MakeHeader(handleStream, kPostArticleHeaderStrings);
			handleStream->Free(); handleStream = nil;
		}

// body
		TFile *file = fPostArticleInfo->fFile;
		FInfo finderInfo;
		FailOSErr(file->GetFinderInfo(finderInfo));
		if (finderInfo.fdType != 'TEXT')
			FailOSErr(errCannotPostNonTextFiles);

		FailOSErr(file->OpenFile());

		long headerSize = GetHandleSize(fText);
		long bodySize;
		FailOSErr(file->GetDataLength(bodySize));
		if (gPrefs->GetBooleanPrefs('2022'))
			Export2022Body(bodySize);
		else
		{
			SetPermHandleSize(fText, headerSize + bodySize);
			HLock(fText);
			FailOSErr(file->ReadData(*fText + headerSize, bodySize));
			TranslateViaTable(gMac2NetAscii, *fText + headerSize, bodySize);			
			HUnlock(fText);
		}
		FailOSErr(file->CloseFile());

// signature
		if (fPostArticleInfo->fAddSignatureChoice && !fPostArticleInfo->fEditSignature)
		{
			long size = GetHandleSize(fText);
			while (size > headerSize && *( *fText + size - 1) <= 32) // strip off empty lines at end,
				size--;
			SetPermHandleSize(fText, ++size);
			*( *fText + size - 1) = 13;
			THandleStream *hs = new THandleStream();
			hs->IHandleStream(fText, 256);
			handleStream = hs;
			handleStream->SetPosition(size);
			fPostArticleInfo->MakeSignature(handleStream);
			handleStream->Free(); handleStream = nil;
		}
		long size = GetHandleSize(fText);
		while (size > headerSize && *( *fText + size - 1) <= 32) // strip off empty lines at end,
			size--;
		SetPermHandleSize(fText, ++size);
		*( *fText + size - 1) = 13;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(handleStream); handleStream = nil;
		if (fPostArticleInfo->fFile)
			fPostArticleInfo->fFile->CloseFile();
		fi.ReSignal();
	}
}

void TPostArticleCommand::Export2022Body(long bodySize)
{
	Handle fromH = nil;
	VOLATILE(fromH);
	PHandleCodeConverter *converter = nil;
	VOLATILE(converter);
	FailInfo fi;
	if (fi.Try())
	{
		fromH = NewPermHandle(bodySize);
		HLock(fromH);
		FailOSErr(fPostArticleInfo->fFile->ReadData(*fromH, bodySize));
		HUnlock(fromH);
		PHandleCodeConverter *hcc = new PHandleCodeConverter();
		hcc->IHandleCodeConverter(fromH, 1024);
		converter = hcc;
		converter->ConvertMac2Net();
		long headerSize = GetHandleSize(fText);
		long convertedSize = converter->GetOutputSize();
		SetPermHandleSize(fText, headerSize + convertedSize);
		BytesMove(converter->GetOutputPtr(), *fText + headerSize, convertedSize);
		delete converter; converter = nil;
		fromH = DisposeIfHandle(fromH);
		fi.Success();
	}
	else // fail
	{
		delete converter; converter = nil;
		fromH = DisposeIfHandle(fromH);
		fi.ReSignal();
	}
}

void TPostArticleCommand::PostArticle()
{
	FailInfo fi;
	if (fi.Try())
	{
		fPostArticleInfo->CreateNntp();
		gCurProgress->SetText(kPostArticleSendingText);
		fPostArticleInfo->fNntp->PostArticle(fText, kPostArticleACKWait);
		fi.Success();
	}
	else // fail
	{
		fPostArticleInfo->DiscardNntp();
		fi.ReSignal();
	}
}

void TPostArticleCommand::TrashDiskCopy()
{
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec trashSpec, fromSpec, tmpSpec;
		fPostArticleInfo->fFile->GetFileSpec(fromSpec);
		tmpSpec = fromSpec;
		trashSpec = fromSpec;
		FailOSErr(FindFolder(trashSpec.vRefNum, kTrashFolderType, kCreateFolder, 
			trashSpec.vRefNum, trashSpec.parID));
		MakeFilenameUnique(trashSpec);
		CStr255 tmpName;
		while (true)
		{
			gCurThread->YieldTime();
			NumToString(TickCount() & 0x7FFFFFFF, tmpName);
			tmpName.Insert("Nuntius", 1);
			CopyCString2String(tmpName, tmpSpec.name);
			tmpSpec.vRefNum = fromSpec.vRefNum;
			tmpSpec.parID = fromSpec.parID;
			if (FileExist(tmpSpec))
				continue;
			tmpSpec.vRefNum = trashSpec.vRefNum;
			tmpSpec.parID = trashSpec.parID;
			if (FileExist(trashSpec))
				continue;
			break; // forgot this one in first place...
		}
		MyRenameFile(fPostArticleInfo->fFile, tmpName);
		FailOSErr(fPostArticleInfo->fFile->MoveAndRename(trashSpec));
#if 0
		HParamBlockRec pb;
		pb.copyParam.ioVRefNum = fromSpec.vRefNum;
		pb.copyParam.ioDirID = fromSpec.parID;
		pb.copyParam.ioNamePtr = fromSpec.name;
		pb.copyParam.ioNewDirID = trashSpec.parID;
		pb.copyParam.ioNewName = trashSpec.name;
		pb.copyParam.ioCopyName = nil;
		FailOSErr(PBHMoveRenameSync(&pb));
#endif
		fi.Success();
	}
	else // fail
	{
		if (fi.error)
			gApplication->ShowError(fi.error, messageTrashPostedDocument);
	}
}
//----------------------------------------------------------------------
TCancelArticleCommand::TCancelArticleCommand()
{
}

pascal void TCancelArticleCommand::Initialize()
{
	inherited::Initialize();
}

void TCancelArticleCommand::ICancelArticleCommand(const CStr255 &messageID)
{
	PPostArticleInfo *ai = new PPostArticleInfo();
	ai->IPostArticleInfo();
	inherited::IPostArticleCommand(ai);
	ai->fReferences = messageID;
	CStr255 s;
	MyGetIndString(s, kCancelArticleProgressTitle);
	ai->fSubject = s; // ugly ugly
}
	
pascal void TCancelArticleCommand::Free()
{
	inherited::Free();
}

pascal void TCancelArticleCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		inherited::DoIt();
		fi.Success();
	}
	else // fail
	{
		if (fi.message == messageCouldNotPostArticle)
			Failure(fi.error, messageCancelArticle);
		else
			fi.ReSignal();
	}
}

void TCancelArticleCommand::ReadArticle()
{
	THandleStream *handleStream = nil;
	VOLATILE(handleStream);
	FailInfo fi;
	if (fi.Try())
	{
		fText = NewPermHandle(0);
		THandleStream *hs = new THandleStream();
		hs->IHandleStream(fText, 128);
		handleStream = hs;
		fPostArticleInfo->MakeHeader(handleStream, kCancelArticleHeaderStrings);
		handleStream->Free(); handleStream = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(handleStream); handleStream = nil;
		fi.ReSignal();
	}
}

void TCancelArticleCommand::PostArticle()
{
	FailInfo fi;
	if (fi.Try())
	{
		fPostArticleInfo->CreateNntp();
		gCurProgress->SetText(kCancelArticleSendingRequest);
		fPostArticleInfo->fNntp->PostArticle(fText, kPostArticleACKWait);
		fi.Success();
	}
	else // fail
	{
		fPostArticleInfo->DiscardNntp();
		if (fi.error == noErr) // cancel
			StdAlert(phPossiblePostedAnyway);
		fi.ReSignal();
	}
}

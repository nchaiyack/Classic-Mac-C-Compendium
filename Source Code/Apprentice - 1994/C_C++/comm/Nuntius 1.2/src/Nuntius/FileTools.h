// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FileTools.h

#define __FILETOOLS__

void CheckFilenameSanity(CStr255 &filename);

Boolean FileExist(const FSSpec &spec);
Boolean FileExist(TFile *file);

void MakeFilenameUnique(FSSpec &spec);
void MakeFilenameUnique(TFile *file);

void ResolveAliasFile(TFile *file);

void MyRenameFile(TFile *file, const CStr255 &newname);

void FlushVols();

void GoInsideFolder(FSSpec &spec); // creates folder if non-existent
void GoGroupFile(const CStr255 &groupName, FSSpec &spec);

void GetPathNameFromDirID(short vRefNum, long dirID, CStr255 &pathName);

void ChangeFileTypeAndCreator(TFile *file, OSType newCreator, OSType newType);
void AskFileName(const CStr255 &prompt, const CStr255 &defaultFilename, FSSpec &spec);

void WriteASyncToFile(ParamBlockRec &pb, TFile *file, const void *p, long numBytes);
void ReadASyncFromFile(ParamBlockRec &pb, TFile *file, void *p, long &numBytes);

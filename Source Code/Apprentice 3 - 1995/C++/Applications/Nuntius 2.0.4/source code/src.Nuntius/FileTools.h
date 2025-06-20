// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FileTools.h

#define __FILETOOLS__


Boolean FileExist(const FSSpec &spec);
Boolean FileExist(TFile *file);

void CheckFilenameSanity(CStr255 &filename);
void MakeFilenameUnique(FSSpec &spec);
void MakeFilenameUnique(TFile *file);

void ResolveAliasSpec(FSSpec &spec);
void ResolveAliasTFile(TFile *file);

void MyRenameFile(TFile *file, const CStr255 &newname);

void FlushVols();

void GoInsideFolder(FSSpec &spec); // creates folder if non-existent
void GoGroupFile(const CStr255 &groupName, FSSpec &spec);

void GetPathNameFromDirID(short vRefNum, long dirID, CStr255 &pathName);

void ChangeFileTypeAndCreator(TFile *file, OSType newType, OSType newCreator);
void AskFileName(const CStr255 &prompt, const CStr255 &defaultFilename, FSSpec &spec);

//MyFindFolder caches calls to FindFolder
OSErr MyFindFolder(short vRefNum, OSType folderType, Boolean createFolder, short *foundVRefNum, long *foundDirID);
TFileHandler *NewMyFileHandler(TFileBasedDocument *itsDocument, TFile* itsFile);
// Fixes bug in MacApp FileHandler: can't save if FindFolder(tempFolder) fails

TFile *NewTempFile(TFile *dataFile);
void DoPostSaveTmpFile(TFile *dataFile, TFile *tmpFile);
void DoFailedSaveTempFile(TFile *tmpFile);
// When NewTempFile is called, dataFile should point to the folder
// where the file is to be saved!

Handle LoadFileIntoHandle(const FSSpec &spec, Boolean isPICT = false);

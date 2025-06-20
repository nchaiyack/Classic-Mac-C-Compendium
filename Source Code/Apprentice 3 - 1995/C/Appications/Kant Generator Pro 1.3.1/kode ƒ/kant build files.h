#ifndef __MY_BUILD_FILES_H__
#define __MY_BUILD_FILES_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void UseTheModule(FSSpec *theFS, Boolean useDefault);
extern	OSErr GetNamedModuleFS(FSSpec *theFS, Str255 theName);
extern	OSErr BuildModulesList(MenuHandle theMenu);
extern	OSErr BuildReferencesList(MenuHandle theMenu, FSSpec theFS);
extern	OSErr DoNewModule(void);
extern	OSErr OpenTheModule(FSSpec *theFS, Boolean newModule, Boolean useOldFile);
extern	Boolean ReferenceNameExistsQQ(FSSpec theFS, Str255 refName);
extern	OSErr DeleteOneReference(FSSpec theFS, Str255 refName);
extern	OSErr DeleteOneInstantiation(FSSpec theFS, Str255 refName, short stringIndex);
extern	OSErr AddOneReference(FSSpec theFS, Str255 refName);
extern	OSErr AddOneInstantiation(FSSpec theFS, Str255 refName, Str255 instantName);
extern	OSErr ReplaceOneReference(FSSpec theFS, Str255 oldRef, Str255 newRef);
extern	OSErr ReplaceOneInstantiation(FSSpec theFS, Str255 oldRef, short stringIndex, Str255 newStr);
extern	OSErr SetInterestingBit(FSSpec theFS, Str255 referenceName, Boolean isInteresting);

#ifdef __cplusplus
}
#endif

#endif

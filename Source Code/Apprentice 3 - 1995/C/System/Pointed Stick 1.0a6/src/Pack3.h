#ifndef __PACK3__
#define __PACK3__

#include "Remember.h"

enum {kNotFaking, kFaking};

extern UniversalProcPtr	gOldStandardFile;
extern short			gStandardFileMode;
extern FileObjectHdl	gFileToReturn;
extern short			gOffsetToReply[];
extern Boolean			gScarfedItYet;
extern short			gRefNum;
extern short			gSelector;

pascal void			MyPack3(void);
void				MyLSetSelect(Boolean setIt, Cell* originalItem, ListHandle lHandle);
void				MyLScroll(short *dCols, short *dRows, ListHandle lHandle);
short				GoToDirectory(FSSpecPtr spec);
short				GoToFile(FSSpecPtr spec);
Boolean				CanOpenFile(FSSpec *file, OSType *itsType, OSErr *err);


enum {sfPutFile = 1, sfGetFile, sfPPutFile, sfPGetFile, standardPutFile,
		standardGetFile, customPutFile, customGetFile};

typedef pascal void (*SFPutFileProc) (	Point where,
										ConstStr255Param prompt,
										ConstStr255Param origName,
										DlgHookProcPtr dlgHook,
										SFReply *reply,
										short selector);

typedef pascal void (*SFGetFileProc) (	Point where,
										ConstStr255Param prompt,
										FileFilterProcPtr fileFilter,
										short numTypes,
										SFTypeList typeList,
										DlgHookProcPtr dlgHook,
										SFReply *reply,
										short selector);

typedef pascal void (*SFPPutFileProc) (	Point where,
										ConstStr255Param prompt,
										ConstStr255Param origName,
										DlgHookProcPtr dlgHook,
										SFReply *reply,
										short dlgID,
										ModalFilterProcPtr filterProc,
                       					short selector);

typedef pascal void (*SFPGetFileProc) (	Point where,
										ConstStr255Param prompt,
										FileFilterProcPtr fileFilter,
										short numTypes,
										SFTypeList typeList,
										DlgHookProcPtr dlgHook,
										SFReply *reply,
										short dlgID,
										ModalFilterProcPtr filterProc,
										short selector);

typedef pascal void (*StandardPutFileProc) (ConstStr255Param prompt,
											ConstStr255Param defaultName,
											StandardFileReply *reply,
											short selector);

typedef pascal void (*StandardGetFileProc) (FileFilterProcPtr fileFilter,
											short numTypes,
											SFTypeList typeList,
											StandardFileReply *reply,
											short selector);

typedef pascal void (*CustomPutFileProc) (	ConstStr255Param prompt,
											ConstStr255Param defaultName,
											StandardFileReply *reply,
											short dlgID,
											Point where,
											DlgHookYDProcPtr dlgHook,
											ModalFilterYDProcPtr filterProc,
											short *activeList,
											ActivateYDProcPtr activateProc,
											void *yourDataPtr,
											short selector);

typedef pascal void (*CustomGetFileProc) (	FileFilterYDProcPtr fileFilter,
											short numTypes,
											SFTypeList typeList,
											StandardFileReply *reply,
											short dlgID,
											Point where,
											DlgHookYDProcPtr dlgHook,
											ModalFilterYDProcPtr filterProc,
											short *activeList,
											ActivateYDProcPtr activateProc,
											void *yourDataPtr,
											short selector);



typedef struct {
	short				selector;
	SFReply*			reply;
	DlgHookProcPtr		dlgHook;
	ConstStr255Param	origName;
	ConstStr255Param	prompt;
	Point				where;
} SFPutFileParameters;


typedef struct {
	short				selector;
	SFReply*			reply;
	DlgHookProcPtr		dlgHook;
	SFTypeList*			typeList;
	short				numTypes;
	FileFilterProcPtr	fileFilter;
	ConstStr255Param	prompt;
	Point				where;
} SFGetFileParameters;


typedef struct {
	short				selector;
	ModalFilterProcPtr	filterProc;
	short				dlgID;
	SFReply*			reply;
	DlgHookProcPtr		dlgHook;
	ConstStr255Param	origName;
	ConstStr255Param	prompt;
	Point				where;
} SFPPutFileParameters;


typedef struct {
	short				selector;
	ModalFilterProcPtr	filterProc;
	short				dlgID;
	SFReply*			reply;
	DlgHookProcPtr		dlgHook;
	SFTypeList*			typeList;
	short				numTypes;
	FileFilterProcPtr	fileFilter;
	ConstStr255Param	prompt;
	Point				where;
} SFPGetFileParameters;


typedef struct {
	short				selector;
	StandardFileReply*	reply;
	ConstStr255Param	defaultName;
	ConstStr255Param	prompt;
} StandardPutFileParameters;


typedef struct {
	short				selector;
	StandardFileReply*	reply;
	SFTypeList*			typeList;
	short				numTypes;
	FileFilterProcPtr	fileFilter;
} StandardGetFileParameters;


typedef struct {
	short					selector;
	void*					yourDataPtr;
	ActivateYDProcPtr		activateProc;
	short*					activeList;
	ModalFilterYDProcPtr	filterProc;
	DlgHookYDProcPtr		dlgHook;
	Point					where;
	short					dlgID;
	StandardFileReply*		reply;
	ConstStr255Param		defaultName;
	ConstStr255Param		prompt;
} CustomPutFileParameters;


typedef struct {
	short					selector;
	void*					yourDataPtr;
	ActivateYDProcPtr		activateProc;
	short*					activeList;
	ModalFilterYDProcPtr	filterProc;
	DlgHookYDProcPtr		dlgHook;
	Point					where;
	short					dlgID;
	StandardFileReply*		reply;
	SFTypeList*				typeList;
	short					numTypes;
	FileFilterYDProcPtr		fileFilter;
} CustomGetFileParameters;

#endif

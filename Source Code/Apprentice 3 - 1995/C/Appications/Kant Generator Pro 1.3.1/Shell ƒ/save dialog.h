#ifndef __SAVE_DIALOG_H__
#define __SAVE_DIALOG_H__

enum	/* return codes from DisplaySaveAlert */
{
	kUserSaved=0,
	kUserDidntSave,
	kUserCanceled
};

#ifdef __cplusplus
extern "C" {
#endif

extern	short DisplaySaveAlert(WindowRef theWindow, Str255 fileName, Str255 verb);

#ifdef __cplusplus
}
#endif

#endif

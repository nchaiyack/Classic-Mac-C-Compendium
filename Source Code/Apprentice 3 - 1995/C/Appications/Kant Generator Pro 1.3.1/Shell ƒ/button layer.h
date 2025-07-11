#ifndef __MY_BUTTON_LAYER_H__
#define __MY_BUTTON_LAYER_H__

#define kSquareButton			1
#define kRadioButton			2
#define kRoundRectButton		4
#define kButtonIsPopup			32
#define kButtonIsDown			64
#define kButtonIsDimmed			128
#define kButtonIsDefault		256
#define kButtonUseDefaultFont	512

#ifdef __cplusplus
extern "C" {
#endif

extern	void Draw3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, short buttonType);
extern	Boolean Track3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, short buttonType);
extern	void Hit3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, short buttonType);

#ifdef __cplusplus
}
#endif

#endif

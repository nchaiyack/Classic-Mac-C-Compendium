#ifndef __MY_OTHER_PRODUCTS_H__
#define __MY_OTHER_PRODUCTS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheOtherProductsWindow(WindowRef theWindow);
extern	void ShutDownTheOtherProductsWindow(void);
extern	void OpenTheOtherProductsWindow(WindowRef theWindow);
extern	void KeyPressedInOtherProductsWindow(WindowRef theWindow, unsigned char theChar);
extern	void MouseClickedInOtherProductsWindow(WindowRef theWindow, Point thePoint);
extern	void DisposeTheOtherProductsWindow(WindowRef theWindow);
extern	void ResizeControlsInOtherProductsWindow(WindowRef theWindow);
extern	void GetGrowSizeOtherProductsWindow(WindowRef theWindow, Rect *sizeRect);

#ifdef __cplusplus
}
#endif

#endif

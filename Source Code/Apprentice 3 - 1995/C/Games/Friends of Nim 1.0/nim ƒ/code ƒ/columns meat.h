#ifndef __COLUMNS_MEAT_H__
#define __COLUMNS_MEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitColumns(void);
extern	void InitColumnsOneGame(void);
extern	void ColumnsDrawWindow(WindowRef theWindow, short theDepth);
extern	void ColumnsClick(WindowRef theWindow, Point thePoint);
extern	void ColumnsIdle(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif

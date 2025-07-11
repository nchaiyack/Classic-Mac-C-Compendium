#ifndef __NIM_GRID_H__
#define __NIM_GRID_H__

#define kGridRowHeight			19
#define kGridColumnWidth		19
#define kLargeGridRowHeight		29
#define kLargeGridColumnWidth	29

#ifdef __cplusplus
extern "C" {
#endif

extern	void DrawGridPosition(Boolean isLarge, Rect *boundsRect, Point pos, Boolean player, short maxRows);
extern	void DrawNeutralGridPosition(Boolean isLarge, Rect *boundsRect, Point pos, short maxRows);
extern	void DrawGrid(Boolean isLarge, short theDepth, Rect *boundsRect, short maxRow, short maxCol);
extern	Point PositionFromPoint(Boolean isLarge, Point thePoint, Rect *boundsRect, short maxRows);
extern	void RectFromPosition(Boolean isLarge, Point pos, Rect *boundsRect, Rect *gridRect, short maxRows);

#ifdef __cplusplus
}
#endif

#endif

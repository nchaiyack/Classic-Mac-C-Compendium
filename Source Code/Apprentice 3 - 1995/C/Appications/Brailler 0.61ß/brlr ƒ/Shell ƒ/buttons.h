void Draw3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, Boolean isDown, Boolean isDimmed, Boolean drawTriangle);
Boolean Track3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, Boolean drawTriangle);
void Hit3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, Boolean drawTriangle);

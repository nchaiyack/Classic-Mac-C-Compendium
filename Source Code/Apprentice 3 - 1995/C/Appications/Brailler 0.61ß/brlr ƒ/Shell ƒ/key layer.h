#ifndef __KEY_LAYER_H__
#define __KEY_LAYER_H__

enum
{
	key_Home=0x01, key_Enter=0x03, key_End, key_Delete=0x08, key_Tab, key_PageUp=0x0b,
	key_PageDown, key_Return, key_Escape=0x1b, key_LeftArrow, key_RightArrow, key_UpArrow,
	key_DownArrow
};

void SaveEventModifiers(EventRecord* theEvent);
short GetTheModifiers(void);
Boolean CommandKeyWasDown(void);
Boolean OptionKeyWasDown(void);
Boolean ShiftKeyWasDown(void);

#endif

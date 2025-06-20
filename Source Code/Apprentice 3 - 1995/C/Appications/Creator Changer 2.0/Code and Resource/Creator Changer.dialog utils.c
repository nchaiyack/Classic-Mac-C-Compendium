/**********************************************************************
 *	This file contains all of the functions which are utilities to
 *	the dialog boxes.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.dialog utils.h"


/**********************************************************************
 *	Function Draw_Default_Border(), this function draws the border
 *	around the item sent to it, that is within the specified dialog box
 **********************************************************************/

void Draw_Border(DialogPtr the_dialog, short the_item, short is_default)
	{
	
	Handle		the_handle;
	short		the_type;
	Rect		the_rect;
	GrafPtr		old_port;
	short		diameter;
			
	GetPort(&old_port);
	SetPort(the_dialog);

	GetDItem(the_dialog, the_item, &the_type, &the_handle, &the_rect);

	if(is_default)
		{
		diameter=(the_rect.bottom-the_rect.top)/2+6;
		if(diameter<16) diameter=16;
		PenSize(3, 3);
		InsetRect(&the_rect, -4, -4);
		FrameRoundRect(&the_rect, diameter, diameter);
		}
	else
		{
		PenSize(1, 1);
		FrameRect(&the_rect);
		}
	
	SetPort(old_port);
	
	}



/**********************************************************************
 *	Function My_Dialog_Filter(), this function handels if there was a 
 *	particular key pressed in the dialog box, and if the dialog box was
 *	moved at all.
 **********************************************************************/
 
pascal Boolean My_Dialog_Filter(DialogPtr the_dialog, EventRecord *the_event, short *item_hit)
	{
	
	Boolean	answer=FALSE;
	
	switch(the_event->what)
		{
		case updateEvt:
			{
			GrafPtr old_port;

			GetPort(&old_port);
			SetPort(the_dialog);
			
			BeginUpdate(the_dialog);
				UpdtDialog(the_dialog, the_dialog->visRgn);
				if(the_dialog==Chng_Dialog)
					{
					Draw_Dialog();
					Draw_Border(Chng_Dialog, CHNG_OK, YES);
					}
				else if(the_dialog==Edit_Dialog) Draw_Border(Edit_Dialog, EDIT_CLOSE, YES);
			EndUpdate(the_dialog);
			
			SetPort(old_port);
			answer=TRUE;
			}
			break;
		case mouseDown:
			{
			WindowPtr		the_window;
			RgnHandle		the_gray_rgn;
			short			the_part;
			static Point	where;

			where=the_event->where;
			
			the_part=FindWindow(where, &the_window);
			if((the_part==inDrag) && (the_window==the_dialog))
				{
				the_gray_rgn=GetGrayRgn();
				DragWindow(the_window, the_event->where, &((**the_gray_rgn).rgnBBox));
				answer=TRUE;
				}
			else GlobalToLocal(&where);
			}
			break;
		case keyDown:
			{
			static char	key_pressed;
			
			key_pressed=the_event->message & charCodeMask;
			
			if(!(the_event->modifiers & cmdKey))
				switch(key_pressed)
					{
					case Escape_Key:
						if(the_dialog==Chng_Dialog)
							{
							Handle_Key_Pressed(the_dialog, CHNG_CANCEL);
							*item_hit=CHNG_CANCEL;
							answer=TRUE;
							}
						else if(the_dialog==Edit_Dialog)
							{
							Handle_Key_Pressed(the_dialog, EDIT_CLOSE);
							*item_hit=EDIT_CLOSE;
							answer=TRUE;
							}
						break;
					case Return_Key:
					case Enter_Key:
						if(the_dialog==Chng_Dialog)
							{
							Handle_Key_Pressed(the_dialog, CHNG_OK);
							*item_hit=CHNG_OK;
							answer=TRUE;
							}
						else if(the_dialog==Edit_Dialog)
							{
							Handle_Key_Pressed(the_dialog, EDIT_CLOSE);
							*item_hit=EDIT_CLOSE;
							answer=TRUE;
							}
						break;
					}
			else
				{
				if(the_dialog==Chng_Dialog)
					switch(key_pressed)
						{
						case 'M':
						case 'm':
							Handle_Key_Pressed(the_dialog, CHNG_MKLK);
							*item_hit=CHNG_MKLK;
							answer=TRUE;
							break;
						case 'C':
						case 'c':
							Handle_Key_Pressed(the_dialog, CHNG_CANCEL);
							*item_hit=CHNG_CANCEL;
							answer=TRUE;
							break;
						}
				else if(the_dialog==Edit_Dialog)
					switch(key_pressed)
						{
						case 'D':
						case 'd':
							Handle_Key_Pressed(the_dialog, EDIT_DELETE);
							*item_hit=EDIT_DELETE;
							answer=TRUE;
							break;
						case 'A':
						case 'a':
							Handle_Key_Pressed(the_dialog, EDIT_ADD);
							*item_hit=EDIT_ADD;
							answer=TRUE;
							break;
						case 'G':
						case 'g':
							Handle_Key_Pressed(the_dialog, EDIT_GET_FILE);
							*item_hit=EDIT_GET_FILE;
							answer=TRUE;
							break;
						}
				}
			}
			break;
		}
	return(answer);
	}



/**********************************************************************
 *	Function Handle_Key_Pressed(), this function handels what happens 
 *	if a key is pressed.
 **********************************************************************/
 
void Handle_Key_Pressed(DialogPtr the_dialog, int the_item)
	{
	
	short	item_type, item_value;
	Handle	item_handle;
	Rect	item_rect;
	long	delay;
					
	GetDItem(the_dialog, the_item, &item_type, &item_handle, &item_rect);
	HiliteControl((ControlHandle)item_handle, 1);
	Delay(DELAY, &delay);
	HiliteControl((ControlHandle)item_handle, 0);
	
	}



/**********************************************************************
 *	Function Check_Data(), this function checks the type entered
 *	by the user.  If there are more than 4 charachters than there is 
 *	an error, and if there is no data entered in there is an error.
 **********************************************************************/

int Check_Data(DialogPtr the_dialog, int ID, short is_desc)
	{
	
	Str255		the_error;
	Str32		item_text;
	Handle		item_handle;
	short		item_type;
	Rect		item_rect;
	int			len;
	short		the_type_len;
	
	GetDItem(the_dialog, ID, &item_type, &item_handle, &item_rect);
	GetIText(item_handle, item_text);
	
	len=item_text[0];
	if(is_desc) the_type_len=DESC_LEN;
	else the_type_len=TYPE_LEN;
	if(len!=the_type_len && the_type_len==TYPE_LEN) return(TYPE_LEN);
	else if((len>the_type_len && the_type_len==DESC_LEN) || len==0) return(DESC_LEN);
	else return(GOOD);
	
	}



/**********************************************************************
 *	Function Set_DLOG_Text_Item(), this function sets text boxes in the
 *	selected dialog to the text which is sent to it.
 **********************************************************************/

void Set_DLOG_Text_Item(DialogPtr the_dialog, short ID, StringPtr the_text)
	{
	
	Handle		the_handle;
	Rect		the_rect;
	short		item_type;
	
	GetDItem(the_dialog, ID, &item_type, &the_handle, &the_rect);
	SetIText(the_handle, the_text);
	
	}



/**********************************************************************
 *	Function Draw_Dialog(), this function draws the objects in the
 *	main dialog box.
 **********************************************************************/

void Draw_Dialog(void)
	{
	
	Handle		the_handle;
	Rect		the_rect;
	short		item_type;
	
	TextFont(systemFont);
	TextSize(12);
	TextFace(0);
	
	Draw_Border(Chng_Dialog, CHNG_CRNT_CREATOR, NO);
	Draw_Border(Chng_Dialog, CHNG_CRNT_TYPE,    NO);
	
	if(!Multiple_Files)
		{
		GetDItem(Chng_Dialog, CHNG_CRNT_CREATOR, &item_type, &the_handle, &the_rect);
		MoveTo(the_rect.left+4, the_rect.bottom-7);
		DrawString(C_Type.TEXT);
	
		GetDItem(Chng_Dialog, CHNG_CRNT_TYPE, &item_type, &the_handle, &the_rect);
		MoveTo(the_rect.left+4, the_rect.bottom-7);
		DrawString(F_Type.TEXT);
		}
		
	}


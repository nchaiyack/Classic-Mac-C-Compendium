/* DoNullEvent.c */

extern Boolean gDisk;

void DoNullEvent( void );

void DoNullEvent()
{
	Boolean tempDisk;
	
	tempDisk = IsDisk();
	
	if( tempDisk != gDisk )
	{
		if( tempDisk == false )
			gDisk = false;
		else
		{		
			SysBeep(0);	
			if( ValidateUser() )
				gDisk = true;
			else
			{
				AlertSound();
				WrongDialog();
				EjectDisk();
				
				while( IsDisk() )
					;
					
				gDisk = false;
			}
		}
	}	
}

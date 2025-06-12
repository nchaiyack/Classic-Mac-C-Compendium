pascal void Show_help( short info_id,
	pascal void (*Handle_update)( WindowPtr what_window ) );

pascal void Flash_button( DialogPtr the_dialog, short item_number );

pascal void Fake_ModalDialog( ModalFilterProcPtr filterProc,
								short *itemHit );

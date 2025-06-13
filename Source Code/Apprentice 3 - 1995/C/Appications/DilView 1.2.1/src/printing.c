/* printing.c */

/* note: I know the printing code is messy.. I only changed a few things and left most of the
/* kludges, but it works.  I'll clean it up I get a raise. [da] */

#include <PrintTraps.h>
#include "printing.h"
#include "list.h"
#include "dil.h"
#include "globals.h"
#include "error.h"
#include "constants.h"
#include "status_bar.h"

THPrint gTHPrint;
TPPrPort print_port;
dil_rec *print_dil;

void myInitPrinting( void )
{
	gTHPrint = (THPrint) NewHandle( sizeof(TPrint) );
	if( gTHPrint == nil )
		myError( "Failed to initialize print record.  Try increasing memory allocation.", true );
}

void myDisposePrinting( void )
{
	DisposeHandle( gTHPrint );
}

void myPageSetup( void )
{	
	PrOpen();
	HLock( gTHPrint );
	
	// not really nec, but I feel better..
	PrValidate( gTHPrint );
	
	// do the page setup dialog and remember changes in gTHPrint!
	PrStlDialog( gTHPrint );
	
	HUnlock( gTHPrint );
	PrClose();	
}

void myPrint( short mode )
{
	short first_page, last_page, num_copies;
	short cur_page, cur_copy;
	TPrStatus print_status;
	Boolean more_to_go = true;
	short sluff_dils;
	
	PrOpen();
	HLock( gTHPrint );

	// not really nec, but I feel better..
	PrValidate( gTHPrint );
	
	// put up the print dialog, returns true if the user didn't cancel
	if( PrJobDialog( gTHPrint ))
	{
		num_copies = (**gTHPrint).prJob.iCopies;
		first_page = (**gTHPrint).prJob.iFstPage;
		last_page = (**gTHPrint).prJob.iLstPage;
		
		// can't have 0 or fewer copies..
		if( num_copies < 1 ) num_copies = 1;
		
		if( PrError() != noErr )
		{
			// panic!
			ErrorDLOG( "Printing Error!", true );
		}
		
		// get the first dil we should print
		switch( mode )
		{
			case k_print_cur:
				print_dil = GetCurDil();
				break;
			case k_print_marked:
				print_dil = GetFirstDil();
				if( print_dil->marked == false )
					print_dil = GetNextMarkedDil( print_dil );
				break;
			case k_print_all:
				print_dil = GetFirstDil();
				break;
		}
		
		// obey lower bound..
		sluff_dils = (first_page - 1) * DilsPerPage();
		while( sluff_dils > 0 )
		{
			switch( mode )
			{
				case k_print_cur:
					print_dil = nil;
					break;
				case k_print_marked:
					print_dil = GetNextMarkedDil( print_dil );
					break;
				case k_print_all:
					print_dil = GetNextDil( print_dil );
					break;
			}
			
			sluff_dils--;
		}
					
		for( cur_copy = 0 ;  cur_copy < num_copies ; cur_copy++ )
		{
			for( cur_page = first_page ; cur_page <= last_page ; cur_page++ )
			{
				// at the onset and when memory fills up, lose the old & create a new struct
				if( (cur_page - first_page) % iPFMaxPgs == 0 )
				{
					// if there's old stuff around, send it to press and dispose
					if( cur_page != first_page )
					{
						PrCloseDoc( print_port );
						if( ((**gTHPrint).prJob.bJDocLoop == bSpoolLoop) && (PrError() == noErr) )
							PrPicFile( gTHPrint, 0, 0, 0, &print_status );
					}
					print_port = PrOpenDoc( gTHPrint, nil, nil );
				}
				
				if( PrError() == noErr )
				{
					PrOpenPage( print_port, nil );
					if( PrError() == noErr )
					{
						// draw page!
	
						more_to_go = PrintPage( mode, (**gTHPrint).prInfo.iVRes, 
											&((**gTHPrint).prInfo.rPage), &print_port );	
											
						PrClosePage( print_port );
					}
				} // end of page	
				
				if( !more_to_go )
					cur_page = last_page;
				
			} // end of doc		
	
			PrCloseDoc( print_port );
			
			// spool the file if nec			
			if( ((**gTHPrint).prJob.bJDocLoop == bSpoolLoop) && (PrError() == noErr) )
				PrPicFile( gTHPrint, 0, 0, 0, &print_status );
		} // end of all copies
	}
	HUnlock( gTHPrint );
	PrClose();
	
	// restore the normal status bar
	DrawStatusBar();
}

// print_dil should contain the next dil to be printed!! at onset it should be the first
// appropriate type, then this takes over
Boolean PrintPage( short mode, short resolution, Rect *rPage, TPPrPort *oprint_port )
{
	Rect dil_rect;
	Boolean all_gone = false;
	short i = 0;
	short dils_per_page;
	float dil_block_height;
	float scale;
		
	scale = ((float)resolution) * (1.0/72.0) * ((float)k_print_scale); 
	
	dil_block_height = ((float)k_dil_height) * scale + ((float)k_print_text_size);
	
	dils_per_page = ((float)(rPage->bottom - rPage->top - k_print_footer_size)) /
		 dil_block_height;
	
	MoveTo( rPage->left, rPage->bottom );
	Move( 0, -2 );
	TextFont( k_print_footer_font );
	TextSize( k_print_footer_size );
	DrawString( k_print_footer );
	
	TextFont( k_print_text_font );
	TextSize( k_print_text_size );
	
	while( (i < dils_per_page) && !all_gone )
	{
		if( print_dil == nil )
			all_gone = true;
		else
		{
			SetRect( &dil_rect, rPage->left,
								rPage->top + (i * dil_block_height) + k_print_text_size,
								rPage->left + (k_dil_width * scale),
								rPage->top + ((i+1) * dil_block_height) );
			
			DrawDilInRect( print_dil, &dil_rect );
		
			MoveTo( rPage->left, dil_rect.top );
			Move( 0, 0 ); // offset for text?
			DrawString( print_dil->fname );
			
			// get the next dil
			switch( mode )
			{
				case k_print_cur:
					print_dil = nil;
					break;
				case k_print_marked:
					print_dil = GetNextMarkedDil( print_dil );
					break;
				case k_print_all:
					print_dil = GetNextDil( print_dil );
					break;
			}
			i++;
		}
	}
	// if we ran out, return false so we don't overshoot the end!
	if( all_gone || (print_dil == nil) )
		return( false );
	else
		return( true );
}

short DilsPerPage( void )
//
// note: gTHPrint had better be valid!!
//
{
	Rect *rPage;
	float scale, dil_block_height;
	short dils_per_page, resolution;
	
	rPage = &((**gTHPrint).prInfo.rPage);
	
	resolution = (**gTHPrint).prInfo.iVRes;

	scale = ((float)resolution) * (1.0/72.0) * ((float)k_print_scale); 
	
	dil_block_height = ((float)k_dil_height) * scale + ((float)k_print_text_size);
	
	dils_per_page = ((float)(rPage->bottom - rPage->top - k_print_footer_size)) /
		 dil_block_height;

	return( dils_per_page );
}

#include <PrintTraps.h>
#include "mac-specific.h"

void Maximize_resolution( void );
void Reset_resolution( void );


void Maximize_resolution( void )
{
	TSetRslBlk 	set_res_block;
	TGetRslBlk	get_res_block;
	short		best_res;
	register short		i;
	
	get_res_block.iOpCode = getRslDataOp;
	PrGeneral( (Ptr) &get_res_block );
	if ( (PrError() != noErr) || (get_res_block.iError != noErr) )
	{
		Show_error( "The maximum resolution setting will not work "
		" for this printer driver." );
		return;
	}
	best_res = 0;
	for (i = 0; i < get_res_block.iRslRecCnt; i++)
		if ( (get_res_block.rgRslRec[i].iXRsl ==
			get_res_block.rgRslRec[i].iYRsl) &&
			(get_res_block.rgRslRec[i].iXRsl > best_res) )
			best_res = get_res_block.rgRslRec[i].iXRsl;
	if (best_res > 0)
	{
		set_res_block.iOpCode = setRslOp;
		set_res_block.hPrint = g_print_rec_h;
		set_res_block.iXRsl = best_res;
		set_res_block.iYRsl = best_res;
		PrGeneral( (Ptr) &set_res_block ); /* set device resolution */
		if ( (PrError() != noErr) || (set_res_block.iError != noErr) )
		{
			Show_error( "The maximum resolution setting will not work "
			" for this printer driver." );
		}
	}
}


void Reset_resolution( void )
{
	TSetRslBlk 	set_res_block;
	
	set_res_block.iOpCode = setRslOp;
	set_res_block.hPrint = g_print_rec_h;
	set_res_block.iXRsl = 0;
	set_res_block.iYRsl = 0;
	PrGeneral( (Ptr) &set_res_block ); /* set device resolution */
}
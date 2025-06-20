/* resources.h */

#ifndef __dil_structs__
 #include "structs.h"
#endif

#define k_winloc_rsrc_type	'data'
#define k_winloc_rsrc_id	128
#define k_scratch_rsrc_file_name "\pDilView_scratch_file"
#define k_DilView_creator_type 'DlV3'
#define k_DilView_scratch_file_type 'temp'

void UseApplRes( void );
void myInitResource( void );
void myDisposeResource( void );
void StoreWinLoc( void );
void RestoreWinLoc( void );
short PicHandleToPICT( PicHandle pic );
void CreateScratchFile( void );
void DeleteScratchFile( void );
void myInitScratchFile( void );
void myDisposeScratchFile( void );
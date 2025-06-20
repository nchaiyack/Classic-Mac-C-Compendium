/**********************************************************************

    Section: FnErr_Library.c

***********************************************************************/

/********** Defines */
#define ERROR_ALERT 900
#define FATAL_ERROR "\pFatal Error!"
#define GENERAL_ERR 900
#define BAD_SYS     901
#define NO_RESOURSE 902
#define AE_ERROR    903
#define NIL_STR     "\p"
#define NIL_PTR     0L

/********** Prototypes */
void FnErr_DisplayStrID( int    stringNum,
                         int    quitFlag );
void FnErr_DisplayStr(   Str255 s1,
                         Str255 s2,
                         Str255 s3,
                         Str255 s4,
                         int    quitFlag );

/********** DisplayStrID */

void FnErr_DisplayStrID( int stringNum, int quitFlag )
{
    StringHandle   errorStringH;
    
    if ( ( errorStringH = GetString( stringNum ) ) == NIL_PTR )
        ParamText( FATAL_ERROR, NIL_STR, NIL_STR, NIL_STR );
    else
    {
        HLock( (Handle)errorStringH );
        ParamText( *errorStringH, NIL_STR, NIL_STR, NIL_STR );
        HUnlock( (Handle)errorStringH );
    }
    StopAlert( ERROR_ALERT, NIL_PTR );
    if( quitFlag )
        ExitToShell();
}

/********** DisplayStr */

void FnErr_DisplayStr(   Str255 s1,
                         Str255 s2,
                         Str255 s3,
                         Str255 s4,
                         int    quitFlag )
{
    ParamText( s1, s2, s3, s4 );
    StopAlert( ERROR_ALERT, NIL_PTR );
    if( quitFlag )
        ExitToShell();
}

// End of File
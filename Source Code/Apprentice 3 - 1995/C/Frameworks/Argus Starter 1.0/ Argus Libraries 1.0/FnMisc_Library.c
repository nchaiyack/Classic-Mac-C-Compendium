/**********************************************************************

    FnMisc_Library.c

***********************************************************************/

/*  Prototypes */

long    FnMisc_ReadPrefs         ( int prefStrID );
void    FnMisc_SavePrefs         ( int prefStrID, long value );
Boolean FnMisc_ColorAvailability ( void );
int     FnMisc_GetPixelDepth     ( GDHandle theDevice );
void    FnMisc_FrameButton       ( DialogPtr theDialog,
                                   short buttonID );


/********** ReadPrefs */

long FnMisc_ReadPrefs( int prefStrID )
/*
    Reads in a string from resource fork specified by prefStrID, 
    converts it to an integer, and returns result.
*/
{
    StringHandle  prefStrH;
    Str255        prefStr;
    unsigned char *tempStr;
    int           strLength, defaultResult, i;
    long          result;
    
    defaultResult = 0;
    if( (prefStrH = GetString( prefStrID )) == NULL )
    {
        result = defaultResult;
    }
    else
    {
        HLock( (Handle)prefStrH );
        strLength = (int)(**prefStrH);
        tempStr = *prefStrH;
        for( i=0; i<=strLength; i++ )
        {
            prefStr[i] = tempStr[i];
        }
        StringToNum( prefStr, &result );
        HUnlock( (Handle)prefStrH );
    }
    return result;
}


/********** SavePrefs */

void FnMisc_SavePrefs( int prefStrID, long value )
/*
    Takes a value, converts it into an string, and saves it into an
    existing resource 'STR' identified by prefStrID.  The length of
    the string must be less than or equal to the existing string
    length.  If smaller, string is padded with leading spaces so
    string length in resource is left unchanged.
*/
{
    StringHandle  prefStrH;
    Str255        prefStr, valueStr;
    unsigned char *tempStr;
    int           strLength, numLength, i;

    if( (prefStrH = GetString( prefStrID )) == NULL )
    {
        // do nothing, string doesn't exist (add error routine?)
    }
    else
    {
        HLock( (Handle)prefStrH );
        strLength = (int)(**prefStrH);
        tempStr = *prefStrH;
        NumToString( value, valueStr );
        numLength = (int)(*valueStr);
        if( numLength <= strLength )
        {
            for( i=1; i<=strLength; i++ )
                tempStr[i] = ' ';
            for( i=(strLength - numLength + 1); i<=strLength; i++ )
                tempStr[i] = valueStr[i-(strLength-numLength)];
        }
        ChangedResource( (Handle)prefStrH );
        WriteResource( (Handle)prefStrH );
        HUnlock( (Handle)prefStrH );
    }
}


/********** ColorAvailability */

Boolean FnMisc_ColorAvailability( void )
/*
    Checks to see if the current machine supports Color QuickDraw.  Use
    this routine once at the beginning of your program.
*/
{
    SysEnvRec mySystem;

    SysEnvirons( 2, &mySystem );
    return( mySystem.hasColorQD );
}


/********** GetPixelDepth */

int FnMisc_GetPixelDepth( GDHandle theDevice )
/*
    Returns the current pixel depth setting of the machine.  Since the
    user can change the setting of the pixel depth on-the-fly (using 
    the 'Monitor' control panel), this routine should be called each
    time you do any drawing.

    Example Usage:
        GDHandle  gCurrentDevice;
        int       gPixelDepth;

        gCurrentDevice = GetDeviceList();
        gPixelDepth = GetPixelDepth( gCurrentDevice );
*/
{
    PixMapHandle screenPMapH;
    int          pixelDepth;

    screenPMapH = (**theDevice).gdPMap;
    pixelDepth = (**screenPMapH).pixelSize;
    return( pixelDepth );
}


/********** FrameButton */
/*
    Frames a button (usually the OK button) in a dialog.
*/
void FnMisc_FrameButton( DialogPtr theDialog, short buttonID )
{
    const int kButtonFrameInset = -4;
    const int kButtonFrameSize = 3;
    const int kFilletSize = 16;

    short    itemType;
    Rect     itemRect;
    Handle   itemHandle;
    PenState thePnState;
    GrafPtr  oldPort;
    
    GetPort( &oldPort );
    SetPort( theDialog );
    GetDItem( theDialog, buttonID, &itemType, &itemHandle, &itemRect );
    GetPenState( &thePnState );
    PenNormal();
    PenPat( black );
    PenSize( kButtonFrameSize, kButtonFrameSize );
    InsetRect( &itemRect, kButtonFrameInset, kButtonFrameInset );
    FrameRoundRect( &itemRect ,kFilletSize, kFilletSize );
    SetPenState( &thePnState );
    SetPort( oldPort );
}

// End of File
/***********************************************************

        This has worked for me, altough it took a lot of
        investigative coding to discover...

        short           getNumberOfColors( void )
                Returns the number of colors the monitor is set to.
                NOTE: This only checks for up to 256 colors.  If you
                        have 24-bit capability and figure out how
                        to check for that also, post it [and mail it
                        to me :-) ]

                        4/19/94  by Paul Rademacher
                                Dept. of Computer Science
                                West Virginia University
                                paulr@cs.wvu.edu

***********************************************************/

#include <GestaltEqu.h>

#define         kColorModeBW                    128
#define         kColorModeColor4                129
#define         kColorModeColor16               130
#define         kColorModeColor256              131

short           getNumberOfColors( void );

short           getNumberOfColors( void )
{
        GDHandle        device;
        short           mode;
        short           quickDrawVersion;
        long            feature;
        OSErr           err;

        /* Here we get the QuickDraw version of this machine */

        if ( err = Gestalt( gestaltQuickdrawVersion, &feature ) == noErr )
        {
                quickDrawVersion = feature;

                if ( quickDrawVersion < gestalt8BitQD )
                {
                        return( 2 );

                        return;
                }
        }
        else
        {
                /* There was an error with Gestalt() - we'll asume 2 colors */

                return( 2 );
        }

        device = GetGDevice();

        mode =  ( **device ).gdMode;

        switch ( mode )
        {
                case kColorModeBW:
                        return( 2 );
                        break;
                case kColorModeColor4:
                        return( 4 );
                        break;
                case kColorModeColor16:
                        return( 16 );
                        break;
                case kColorModeColor256:
                        return( 256 );
                        break;
                default:        /* Does not check for >256 colors... */
                        return( 2 );
                        break;
        }
}


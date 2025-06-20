/* passwdcheck.c */

#define kAccessKeyStrRsrc 128

Boolean ValidateAccessKey( char *key );

Boolean ValidateAccessKey( char *key )
{
	StringHandle	realkey;
	char*			realptr;
	char			togo;
	
	realkey = GetString( kAccessKeyStrRsrc );
										
	realptr = ((char *) *realkey) +1;

	togo = *((char *) *realkey);
	key++;

	if( *key == 0 )
		return( false );

	while( *key != 0)
	{
		togo--;
		if( *(key++) != *(realptr++) )
			return( false );
	}

	if( togo > 0 )
		return( false );
	
	return( true );
}
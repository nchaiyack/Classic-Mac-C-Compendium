/* validate.c */

void GetPasswd( char * );
Boolean ValidateUser( void );
char *InternalBufferDialog(void);

void GetPasswd( char *passwd )
{
	char	*where;
	
	where = InternalBufferDialog();
	strcpy( passwd, where );
}

Boolean ValidateUser( void )
{
	char		passwd[256];
	Boolean		result;
	
	GetPasswd( passwd );

	result = ValidateAccessKey( &passwd );
	
	return( result );	
}
#include <HyperXcmd.h>

/* 
	ExportText XFCN
	
	Syntax:		ExportText( rsrc_id )
	
				or
				
				ExportText( rsrc_id, file_path )
	
	Find TEXT/styl on the clipboard and save it to a file as TEXT and
	styl resources with resource ID rsrc_id.  If resources with that ID
	already exist, they will be replaced.  In the former case, Standard File
	is used to select the file.
	
	by James W. Walker
		Internet		walkerj@math.scarolina.edu
		CompuServe		76367,2271
		America Online	JWWalker
*/


pascal void main( XCmdPtr param_ptr );
void Return_string( XCmdPtr param_ptr, StringPtr str );

#define		NIL				0L

pascal void main( XCmdPtr param_ptr )
{
	Str255	menu_title, work, path;
	short	res_id;
	long	l_res;
	SFReply	reply;
	Point	where;
	short	refnum;
	OSErr	err;
	Handle	rsrc_h, text_h, styl_h;
	long	data_len, offset;
	
	if ((param_ptr->paramCount < 1) || (param_ptr->paramCount > 2) )
	{
		Return_string(param_ptr, "\pThere must be 1 or 2 parameters.");
		return;
	}
	
	ZeroToPas( param_ptr, *(param_ptr->params[0]), work );
	StringToNum( work, &l_res );
	res_id = (short) l_res;
	
	/*
		See if there are TEXT and styl data on the scrap.
	*/
	text_h = NewHandle(0);
	styl_h = NewHandle(0);
	data_len = GetScrap( text_h, 'TEXT', &offset );
	if (data_len < 0)
	{
		Return_string(param_ptr, "\pNo text on scrap!");
		return;
	}
	data_len = GetScrap( styl_h, 'styl', &offset );
	if (data_len < 0)
	{
		Return_string(param_ptr, "\pNo styled text on scrap!");
		return;
	}
	

	/*
		Next let's open the resource file.
	*/
	switch (param_ptr->paramCount)
	{
		case 1:
			where.h = where.v = 100;
			SFGetFile( where, (StringPtr)"\p", NIL, -1, NIL, NIL, &reply );
			if (!reply.good)
			{
				Return_string(param_ptr, "\p");
				return;
			}
			err = SetVol( NIL, reply.vRefNum );
			if (err != noErr)
			{
				Return_string(param_ptr, "\pSetVol error");
				return;
			}
			refnum = OpenResFile( reply.fName );
			break;
		case 2:
			ZeroToPas( param_ptr, *(param_ptr->params[1]), path );
			refnum = OpenResFile( path );
			break;
	}
	if (refnum == -1)
	{
		err = ResError();
		NumToString( err, work );
		Return_string(param_ptr, work );
		return;
	}
	
	/* 
		If the file already has a TEXT/styl resources of the given ID,
		we need to delete them.
	*/
	rsrc_h =  Get1Resource( 'TEXT', res_id );
	if (rsrc_h != NIL)
	{
		RmveResource( rsrc_h );
		UpdateResFile( refnum );
	}
	rsrc_h =  Get1Resource( 'styl', res_id );
	if (rsrc_h != NIL)
	{
		RmveResource( rsrc_h );
		UpdateResFile( refnum );
	}
	
	
	/*
		Add the new menu resource.
	*/
	AddResource( text_h, 'TEXT', res_id, "\p" );
	AddResource( styl_h, 'styl', res_id, "\p" );

	CloseResFile( refnum );
	Return_string( param_ptr, "\p" );
}

/* ----------------------------------------------------------------- */
void Return_string( XCmdPtr param_ptr, StringPtr str )
{
	StringPtr	return_str;
	
	param_ptr->returnValue = NewHandle( str[0] + 1 );
	if (param_ptr->returnValue == NIL)
		return;
	return_str = (StringPtr) *(param_ptr->returnValue);
	BlockMove( &str[1], return_str, str[0] );
	return_str[str[0]] = '\0';
}

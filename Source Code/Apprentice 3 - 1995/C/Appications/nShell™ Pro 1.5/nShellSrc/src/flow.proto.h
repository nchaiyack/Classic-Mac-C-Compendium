/* ==========================================

	flow.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

int  flow_check( Str255 name );
void flow_if( void );
void flow_then( void );
void flow_else( void );
void flow_endif( void );
void flow_while( void );
void flow_until( void );
void flow_do( void );
void flow_done( void );
void flow( void );
void flist_del_all( InterpH interp );
void flow_no_flow( Boolean *flag );

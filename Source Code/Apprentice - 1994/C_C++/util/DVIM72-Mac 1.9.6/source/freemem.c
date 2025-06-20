
long	App_partitions_total( void );

extern	WindowPtr	g_freemem_window;
extern	Boolean		g_use_sysheap;

typedef enum {
	sm_unknown,
	sm_no_temp_mem,
	sm_temp_mem,
	sm_temp_mem_is_real
} sm_temp_mem_status;

extern sm_temp_mem_status	MF_mem_status;

long		g_last_freemem = 0L;
long		g_last_freesys = 0L;
int			g_bar_length;
long		g_allmem;				/* application memory */
long		g_sysheap_size;

#define BAR_HEIGHT	10
#define SIDE_MARGIN	5
#define BAR_TOP		20
#define SBAR_TOP	40
#define SYSHEAP_MAX	50000L
#define SYSHEAP_THRESHOLD	30000L
#define BASELINESKIP	15

void	Update_memory_indicators( void );
void	Update_freemem( void );
pascal long FreeMemSys( void );

void
Update_freemem()
{
	GrafPtr	save_port;
	Str255	mem_str;
	Rect	myrect;

	if ( ((WindowPeek)g_freemem_window)->visible )
	{
		GetPort( &save_port );
	
		SetPort( g_freemem_window );
		if (g_use_sysheap)
			g_bar_length = (g_freemem_window->portRect.right -
				g_freemem_window->portRect.left - 4*SIDE_MARGIN) / 2;
		else
			g_bar_length = g_freemem_window->portRect.right -
				g_freemem_window->portRect.left - 2*SIDE_MARGIN;
		EraseRgn( g_freemem_window->visRgn );
		MoveTo( SIDE_MARGIN, BASELINESKIP );
		Move( StringWidth("\p9999999"), 0 );
		DrawString( "\p of " );
		g_allmem = StripAddress( (Ptr)CurStackBase ) -
			StripAddress( (Ptr)ApplZone );
		NumToString( g_allmem, mem_str );
		DrawString( mem_str );
		MoveTo( SIDE_MARGIN, 3*BASELINESKIP );
		DrawString("\pApplication memory");

		SetRect( &myrect, SIDE_MARGIN, 2*BASELINESKIP - BAR_HEIGHT,
			SIDE_MARGIN + g_bar_length, 2*BASELINESKIP );
		InsetRect( &myrect, -1, -1 );
		FrameRect( &myrect );

		if (g_use_sysheap)
		{
			MoveTo( 3*SIDE_MARGIN + g_bar_length, BASELINESKIP );
			Move( StringWidth("\p9999999"), 0 );
			DrawString( "\p of " );
			g_sysheap_size = StripAddress( MFTopMem() ) -
				StripAddress((Ptr)SysZone->bkLim) - App_partitions_total();
			NumToString( g_sysheap_size, mem_str );
			DrawString( mem_str );
			MoveTo( 3*SIDE_MARGIN + g_bar_length, 3*BASELINESKIP );
			DrawString("\pTemporary Memory");
			OffsetRect (&myrect, g_bar_length + 2*SIDE_MARGIN, 0 );
			FrameRect( &myrect );
		}
		
		g_last_freemem = 0; /* only temporary, to force an update */
		Update_memory_indicators();

		SetPort( save_port );
	}
}

/* --------------------- Update_memory_indicators -------------------- */
/* 
	This routine is invoked when the memory indicators need to be updated.
	It does this with a minimal amount of drawing, and does NOT suffice
	as a response to an update event.
*/
void
Update_memory_indicators()
{
	
	GrafPtr	save_port;
	register long		freemem, freesys;
	long	purgemem, contig;
	Str255	mem_str;
	Rect	myrect;

	if ( ((WindowPeek)g_freemem_window)->visible )
	{
		PurgeSpace( &purgemem, &contig );
		freemem = FreeMem();
		if (g_use_sysheap)
			freesys = MFFreeMem();
		else
			freesys = 0L;
		if ( (freemem != g_last_freemem) || 
			(g_use_sysheap && (freesys != g_last_freesys)) )
		{
			GetPort( &save_port );
		
			SetPort( g_freemem_window );
	
			/* Redraw numerical indication of free memory */
			SetRect( &myrect, SIDE_MARGIN, 0,
				SIDE_MARGIN + StringWidth("\p9999999"),
				2*BASELINESKIP - BAR_HEIGHT - 1 );
			EraseRect( &myrect );
			MoveTo( SIDE_MARGIN, BASELINESKIP );
			NumToString( freemem, mem_str );
			DrawString( mem_str );
			
			/* Adjust free memory bar */
			myrect.bottom = 2*BASELINESKIP;
			myrect.top = myrect.bottom - BAR_HEIGHT;
			myrect.right = myrect.left +
				( (float)freemem / (float)g_allmem ) * g_bar_length;
			FillRect( &myrect, dkGray );
			myrect.left = myrect.right;
			myrect.right = myrect.left +
				( (float)(purgemem - freemem) / (float)g_allmem )
					* g_bar_length;
			FillRect( &myrect, ltGray );
			myrect.left = myrect.right;
			myrect.right = SIDE_MARGIN + g_bar_length;
			EraseRect( &myrect );
			
			if (g_use_sysheap)
			{
				myrect.left = 3*SIDE_MARGIN + g_bar_length;
				if (freesys > g_sysheap_size)
					freesys = g_sysheap_size;
				myrect.right = myrect.left +
					(float)freesys / (float)g_sysheap_size * g_bar_length;
				FillRect( &myrect, dkGray );
				myrect.left = myrect.right;
				myrect.right = 3*SIDE_MARGIN + 2*g_bar_length;
				EraseRect( &myrect );
				
				SetRect( &myrect, 3*SIDE_MARGIN + g_bar_length, 0,
					3*SIDE_MARGIN + g_bar_length +
					StringWidth("\p9999999 of 9999999"),
					2*BASELINESKIP - BAR_HEIGHT - 1 );
				EraseRect( &myrect );
				MoveTo( 3*SIDE_MARGIN + g_bar_length, BASELINESKIP );
				NumToString( freesys, mem_str );
				DrawString( mem_str );
				MoveTo( 3*SIDE_MARGIN + g_bar_length +
					StringWidth("\p9999999"),
					BASELINESKIP );
				DrawString("\p of ");
				NumToString( g_sysheap_size, mem_str );
				DrawString( mem_str );
			}

			SetPort( save_port );
			g_last_freemem = freemem;
			g_last_freesys = freesys;
		}
	}
}

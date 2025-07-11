/* printing.h */

#include <PrintTraps.h>

// the % of actual size for printing (ideal yield = 5 per page) 
// 0.47 = 5 per page
#define k_print_scale 0.47

enum {
	k_print_cur,
	k_print_marked,
	k_print_all
};

#define k_print_text_font	times
#define k_print_text_size	12
#define k_print_footer_font helvetica
#define k_print_footer_size 9
#define k_print_footer		"\pDilbert is � Scott Adams, United Feature Syndicate, and ClariNet"

void myInitPrinting( void );
void myDisposePrinting( void );
void myPageSetup( void );
void myPrint( short mode );
Boolean PrintPage( short mode, short resolution, Rect *rPage, TPPrPort *oprint_port );
short DilsPerPage( void );

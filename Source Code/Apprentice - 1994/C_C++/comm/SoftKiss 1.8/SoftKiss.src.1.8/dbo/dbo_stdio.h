/*
 * dbo_stdout.h - debugging print system
 * by Aaron Wohl
 * Public domain
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15208
 * home: 412-731-6159
 * work: 412-268-5032
 */

#include <stdarg.h>

#ifndef DBO_ENABLED
#define DBO_printf(xx_arg)
#define DBO_fprintf(xx_arg)
#define DBO_CLEAR
#define dbo_fopen(xx_arg)
#define dbo_fputc(xx_arg,xx_arg2)
#define dbo_goto(xx_arg1,xx_arg2)
#define dbo_fgoto(xx_arg1,xx_arg2,xx_arg3)
#define dbo_fclear(xx_arg)
#else
#define dbo_stdout (0L)			/*default output window*/

#define DBO_printf(xx_arg) dbo_printf xx_arg
#define DBO_fprintf(xx_arg) dbo_fprintf xx_arg
#define DBO_CLEAR dbo_clear()

/*
 * constants at start/end of file record
 * for consistancy/overwrite checking
 */
#define dbo_GUARD1 (0x48F2)
#define dbo_GUARD2 (0x7235)

/*
 * dbo equivilant of FILE
 */
struct dbo_FILE_R {
	unsigned short dbo_guard1;	/*constant so we know this is inited ok*/
	int just_kidding;			/*don't print anything*/
	unsigned char *mem;			/*address of screen*/
	unsigned long row_bytes;	/*bytes per row in mem*/
	short nl_extra;				/*extra bytes to clear on new line*/
	short plimit;				/*char output limit per printf*/
	short int_is4;				/*true if integers are 4bytes*/
	Point win_tl;				/*top left address of window*/
	Point win_br;				/*bottom right address of window*/
	Point win_cur;				/*current cursor*/
	unsigned char wrap;			/*wrap output lines if non zero*/
	long resv[4];				/*reserved for internal use/expansion*/
	unsigned short dbo_guard2;	/*constant so we know this is inited ok*/
};
typedef struct dbo_FILE_R dbo_FILE,*dbo_FILE_pt;

/*
 * initialize the passed dbo file for full screen
 * on startup screen
 */
void dbo_fopen(dbo_FILE_pt af);

/*
 * write one character to a file
 */
void dbo_fputc(register dbo_FILE_pt af,char ch);

/*
 * v array version of printf
 */
int dbo_vfprintf(dbo_FILE_pt af,const char *fmt,va_list ap);

/*
 * debugging fprintf
 */
int dbo_fprintf(dbo_FILE *fp,const char *fmt,...);

/*
 * debugging vprintf
 */
int dbo_vprintf(const char *fmt,void *p);

/*
 * debugging printf
 */
int dbo_printf(const char *fmt,...);


/*
 * position cursor in the stdout dbo window
 */
void dbo_goto(int x,int y);

/*
 * position cursor in a dbo window
 */
void dbo_fgoto(dbo_FILE *fp,int x,int y);

/*
 * clear window
 */
void dbo_clear(void);

/*
 * clear file window
 */
void dbo_fclear(dbo_FILE *af);
#endif

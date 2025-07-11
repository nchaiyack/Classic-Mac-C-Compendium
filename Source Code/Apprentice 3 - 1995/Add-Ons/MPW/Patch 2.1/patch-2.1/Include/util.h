/* $Header: util.h,v 2.0 86/09/17 15:40:06 lwall Exp $
 *
 * $Log:	util.h,v $
 * Revision 2.0  86/09/17  15:40:06  lwall
 * Baseline for netwide release.
 * 
 */

// and for those machine that can't handle a variable argument list

#define say1 say
#define say2 say
#define say3 say
#define say4 say
#define ask1 ask
#define ask2 ask
#define ask3 ask
#define ask4 ask
#define fatal1 fatal
#define fatal2 fatal
#define fatal3 fatal
#define fatal4 fatal
#define pfatal1 pfatal
#define pfatal2 pfatal
#define pfatal3 pfatal
#define pfatal4 pfatal


int	move_file	(const char* from, const char* to);
void	copy_file	(const char* from, const char* to);
char*	savestr		(const char* s);
void	say		(const char* format, ...);
void	fatal		(const char* format, ...);
void	pfatal		(const char* format, ...);
void	ask		(const char* format, ...);
void	set_signals	(int reset);
void	ignore_signals	(void);
void	makedirs	(char* filename, bool striplast);
char*	fetchname	(char* at, int strip_leading, int assume_exists);
void*	xmalloc		(size_t size);
void	my_exit		(int status);


#ifdef macintosh
#define	creat(n, a)	creat(n)
#endif

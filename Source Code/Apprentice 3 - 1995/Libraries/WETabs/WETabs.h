/************************************************************************\
	WETabs.h
	
	Hooks for adding tab support to WASTE
	
	Original version by Mark Alldritt
	Line breaking code by Dan Crevier
	Support for horizontal scrolling by Bert Seltzer
	
\************************************************************************/

#define WASTE_TAB_SIZE		30L

#ifdef __cplusplus
extern "C" {
#endif

pascal void _WETabDrawText(Ptr pText, long textLength, Fixed slop, 
				JustStyleCode styleRunPosition, WEHandle hWE);
pascal long _WETabPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE);
pascal short _WETabCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE);
pascal StyledLineBreakCode _WETabLineBreak(Ptr pText, long textLength,
				long textStart, long textEnd, Fixed *textWidth,
				long *textOffset, WEHandle hWE);
pascal void WEInstallTabHooks(WEHandle hWE);

#ifdef __cplusplus
}
#endif

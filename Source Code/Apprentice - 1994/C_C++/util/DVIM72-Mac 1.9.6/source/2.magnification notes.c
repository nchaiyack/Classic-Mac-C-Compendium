#if justacomment

What should magnification mean?  Unmagnified type, at any
printer resolution, should be 1000.

Perhaps we need another concept such as "font resolution". It
would be the product of printer resolution and magnification.
For example, a font file with a resolution of 360 would serve
as a \magstep1 font at 300dpi, or as a magnification = 2500
font at 144dpi.

Globals related to magnification
================================

basemag
	set to g_dpi*5 in init_glob

runmag		"runtime magnification"
	set to basemag in initglob().
	May be reset in option() for -m option.
	This is the guy that gets printed in the log, e.g.,
	[720 magnification] from readpost().

mag_table
	initialized in initglob to powers of sqrt(1.2).

mag_index
	Index into mag_table for nearest magnification to a
	pure factor passed to actfact().

mag		"magnification specified in preamble"


Macros related to magnification
===============================

STDRES
	Set to 0 in m72.h.

STDMAG
	Set to basemag in m72.h.
	In gendefs, set to 603 if STDRES = 0

RESOLUTION
	Set to g_dpi in m72.h.

MAGSIZE
	Converts a pure magnification factor to a 1000-based
	integer magnification.

USEGLOBALMAG	"allow runtime global magnification scaling"
	Set to 0 in machdefs.h, 1 in gendefs.h.
	Used in dvifile(), reldfont(), with value 1.


Routines related to magnification
=================================

actfact
	Rounds a pure magnification factor to nearest step.
	Called in dvifile(), reldfont().

actfact_res
	Rounds a pure magnification factor to nearest step,
	then multiplies by dpi/200.
	
	Called by option(), openfont().

fontfile
	Given a font name and magnification, it returns a list of
	font paths in various formats.  Assumes old-style magnification,
	so it can compute dpi = round(real_mag/5.0).
	
	Using new-style magnification, it should use
		dpi = g_dpi * mag / 1000.  (computed in floating point)
	Maybe that computation should be done in openfont(), so that the
	font-dpi value can be passed to fontfile().
	
	Called by openfont().

openfont
	In charge of finding and opening font files.  Calls fontfile(),
	fontsub(), then looks for neighboring fonts.

#endif
;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst pmCouteous 0)
(defconst pmTolerant 2)
(defconst pmAnimated 4)
(defconst pmExplicit 8)

(defconst pmNoUpdates (hex-string-to-int "8000"))
(defconst pmBkUpdates (hex-string-to-int "a000"))
(defconst pmFgUpdates (hex-string-to-int "c000"))
(defconst pmAllUpdates (hex-string-to-int "e000"))

(c:defstruct ColorInfo ((RGBColor ciRGB)
						(short ciUsage)
						(short ciTolerance)
						((array short 3) ciDataFields)))

(c:defstruct Palette ((short pmEntries)
					  ((array short 7) pmDataFields)
					  ((array ColorInfo 1) pmInfo)))

(c:typedef PalettePtr (ptr . Palette))
(c:typedef PaletteHandle (ptr ptr . Palette))

; pascal PaletteHandle NewPalette(short entries,CTabHandle srcColors,short srcUsage,
;  short srcTolerance)
;  = 0xAA91; 
(deftrap NewPalette ("aa91")
  ((entries short)
   (srcColors long)
   (srcUsage short)
   (srcTolerance short))
  long)

; pascal void DisposePalette(PaletteHandle srcPalette)
;  = 0xAA93; 
(deftrap DisposePalette ("aa93")
  ((srcPalette long))
  nil)

; pascal void ActivatePalette(WindowPtr srcWindow)
;  = 0xAA94; 
(deftrap ActivatePalette ("aa94")
  ((srcWindow long))
  nil)

; pascal PaletteHandle GetPalette(WindowPtr srcWindow)
;  = 0xAA96; 
(deftrap GetPalette ("aa96")
  ((srcWindow address))
  long)

; pascal void CopyPalette(PaletteHandle srcPalette,PaletteHandle dstPalette,
;  short srcEntry,short dstEntry,short dstLength)
;  = 0xAAA1; 
(deftrap CopyPalette ("aaa1")
  ((srcPalette long)
   (dstPalette long)
   (srcEntry short)
   (dstEntry short)
   (dstLength short))
  nil)

; pascal void SetPalette(WindowPtr dstWindow,PaletteHandle srcPalette,Boolean cUpdates)
;  = 0xAA95; 
(deftrap SetPalette ("aa95")
  ((dstWindow long)
   (srcPalette long)
   (cUpdates char))
  nil)

; pascal void GetEntryColor(PaletteHandle srcPalette,short srcEntry,RGBColor *dstRGB)
;  = 0xAA9B; 
(deftrap GetEntryColor ("aa9b")
  ((srcPalette long)
   (srcEntry short)
   (dstRGB address))
  nil)

; pascal void SetEntryColor(PaletteHandle dstPalette,short dstEntry,const RGBColor *srcRGB)
;  = 0xAA9C; 
(deftrap SetEntryColor ("aa9c")
  ((dstPalette long)
   (dstEntry short)
   (srcRGB address))
  nil)

; pascal void NSetPalette(WindowPtr dstWindow,PaletteHandle srcPalette,short nCUpdates)
;  = 0xAA95; 
(deftrap NSetPalette ("aa95")
  ((dstWindow long)
   (srcPalette long)
   (nCUpdates short))
  nil)

; pascal void PmForeColor(short dstEntry)
;  = 0xAA97; 
(deftrap PmForeColor ("aa97")
  ((dstEntry short))
  nil)

; pascal void PmBackColor(short dstEntry)
;  = 0xAA98; 
(deftrap PmBackColor ("aa98")
  ((dstEntry short))
  nil)

; pascal void SetEntryUsage(PaletteHandle dstPalette,short dstEntry,short srcUsage,
;  short srcTolerance)
; = 0xAA9E; 
(deftrap SetEntryUsage ("aa9e")
  ((dstPalette long)
   (dstEntry short)
   (srcUsage short)
   (srcTolerance short))
  nil)

; pascal void AnimateEntry(WindowPtr dstWindow,short dstEntry,const RGBColor *srcRGB)
;  = 0xAA99; 
(deftrap AnimateEntry ("aa99")
  ((dstWindow long)
   (dstEntry short)
   (srcRGB address))
  nil)

; pascal void ResizePalette(PaletteHandle p,short size)
; = {0x7003,0xAAA2}; 
(deftrap ResizePalette ("7003" "aaa2")
  ((p long)
   (size short))
  nil)

; pascal PaletteHandle GetNewPalette(short PaletteID)
;  = 0xAA92; 
(deftrap GetNewPalette ("aa92")
  ((PaletteID short))
  long)

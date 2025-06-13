;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;pascal long ZeroScrap(void)
;    = 0xA9FC; 
(deftrap ZeroScrap ("a9fc")
  ()
  long)

;pascal long PutScrap(long length,ResType theType,Ptr source)
;    = 0xA9FE; 
(deftrap PutScrap ("a9fe")
  ((length long)
   (theType immediate-string)
   (source address))
  long)

;pascal long GetScrap(Handle hDest,ResType theType,long *offset)
;    = 0xA9FD; 
(deftrap GetScrap ("a9fd")
  ((hDest long)
   (theType immediate-string)
   (offset address))
  long)

; pascal PScrapStuff InfoScrap(void)
;    = 0xA9F9; 
(deftrap InfoScrap ("a9f9")
  nil
  long)

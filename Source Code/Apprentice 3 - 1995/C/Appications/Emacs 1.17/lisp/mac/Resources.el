;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

; pascal Handle GetResource(ResType theType,short theID)
;  = 0xA9A0; 
(deftrap GetResource ("a9a0")
  ((theType immediate-string)
   (theID short))
  long)

; pascal void DetachResource(Handle h)
;  = 0xA992; 
(deftrap DetachResource ("a992")
 ((h long))
 nil)

; extern pascal short HomeResFile(Handle theResource)
;  ONEWORDINLINE(0xA9A4);
(deftrap HomeResFile ("a9a4")
  ((h long))
  short)

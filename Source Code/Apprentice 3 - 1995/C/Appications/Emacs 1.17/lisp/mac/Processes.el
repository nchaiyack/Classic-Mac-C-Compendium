;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(c:defstruct ProcessSerialNumber ((long highLongOfPSN)
								  (long lowLongOfPSN)))

(defconst kNoProcess 0)
(defconst kSystemProcess 1)
(defconst kCurrentProcess 2)

; pascal OSErr SetFrontProcess(const ProcessSerialNumber *PSN)
;    = {0x3F3C,0x003B,0xA88F}; 
(deftrap SetFrontProcess ("3f3c" "003b" "a88f")
  ((PSN address))
  short)

; pascal OSErr GetCurrentProcess(const ProcessSerialNumber *PSN)
;    = {0x3F3C,0x0037,0xA88F};
(deftrap GetCurrentProcess ("3F3C" "0037" "A88F")
  ((PSN address))
  short)

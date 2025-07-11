;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst gestaltQuickdrawVersion "qd  ")
(defconst gestalt8BitQD 256 "0x100")

; pascal OSErr Gestalt(OSType selector,long *response)
; = {0xA1AD,0x2288}; 
; #pragma parameter __D0 Gestalt(__D0,__A1)
(deftrap Gestalt ("201f" "a1ad" "2288") ; move.l (a7)+,d0
  ((selector immediate-string)
   (response address a1))
  (short d0))

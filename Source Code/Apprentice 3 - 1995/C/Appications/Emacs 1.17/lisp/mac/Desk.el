;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

; pascal short OpenDeskAcc(ConstStr255Param deskAccName)
;    = 0xA9B6; 
(deftrap OpenDeskAcc ("a9b6")
  ((deskAccName address))
  short)

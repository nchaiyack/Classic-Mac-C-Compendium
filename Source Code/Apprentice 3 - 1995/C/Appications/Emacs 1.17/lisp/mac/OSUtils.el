;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

; pascal OSErr HandToHand(Handle *theHndl); 
(deftrap HandToHand ("2077" "0151"	; move.l ([a7]),a0
					 "a9e1"			; _HandToHand
					 "225f"			; move.l (a7)+,a1
					 "2288"			; move.l a0,(a1)
					 )
  ((theHndl address))
  (short d0))

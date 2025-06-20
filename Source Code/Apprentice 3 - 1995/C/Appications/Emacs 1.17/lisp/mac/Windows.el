;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

; pascal void ShowWindow(WindowPtr theWindow)
; = 0xA915; 
(deftrap ShowWindow ("a915")
  ((theWindow long))
  nil)

; pascal void MoveWindow(WindowPtr theWindow,short hGlobal,short vGlobal,
;    Boolean front)
;    = 0xA91B; 
(deftrap MoveWindow ("a91b")
  ((theWindow long)
   (hGlobal short)
   (vGlobal short)
   (front short))
  nil)

; pascal void InvalRect(const Rect *badRect)
;     = 0xA928; 
(deftrap InvalRect ("a928")
  ((badRect address))
  nil)

; extern pascal void SetWinColor(WindowPtr theWindow, WCTabHandle newColorTable)
;  ONEWORDINLINE(0xAA41);
(deftrap SetWinColor ("aa41")
  ((theWindow long)
   (newColorTable long))
  nil)

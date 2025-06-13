;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst geneva 3)
(defconst monaco 4)
(defconst times 20)
(defconst courier 22)

; pascal void GetFNum(ConstStr255Param name,short *familyID)
;    = 0xA900; 
(deftrap GetFNum ("a900")
  ((name address)
   (familyID address))
  nil)

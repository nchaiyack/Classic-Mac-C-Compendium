;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(deftrap GetColor ("3f3c" "0009" "a82e")
  ((where immediate-string)
   (prompt address)
   (inColor address)
   (outColor address))
  char)

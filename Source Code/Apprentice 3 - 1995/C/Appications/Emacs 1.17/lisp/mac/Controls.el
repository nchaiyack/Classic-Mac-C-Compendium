;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst pushButProc 0)
(defconst checkBoxProc 1)
(defconst radioButProc 2)
(defconst useWFont 8)
(defconst scrollBarProc 16)
(defconst inButton 10)
(defconst inCheckBox 11)
(defconst inUpButton 20)
(defconst inDownButton 21)
(defconst inPageUp 22)
(defconst inPageDown 23)
(defconst inThumb 129)
(defconst popupMenuProc 1008)
(defconst inLabel 1)
(defconst inMenu 2)
(defconst inTriangle 4)

; pascal void HiliteControl(ControlHandle theControl,short hiliteState)
;    = 0xA95D; 
(deftrap HiliteControl ("a95d")
  ((theControl long)
   (hiliteState short))
  nil)

; pascal void SetCtlValue(ControlHandle theControl,short theValue)
;     = 0xA963; 
(deftrap SetCtlValue ("a963")
  ((theControl long)
   (theValue short))
  nil)

; pascal short GetCtlValue(ControlHandle theControl)
;     = 0xA960; 
(deftrap GetCtlValue ("a960")
  ((theControl long))
  short)

;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(c:defstruct EventRecord ((short what)
						  (long message)
						  (long when)
						  (Point where)
						  (short modifiers)))

(defconst nullEvent 0)
(defconst mouseDown 1)
(defconst mouseUp 2)
(defconst keyDown 3)
(defconst keyUp 4)
(defconst autoKey 5)
(defconst updateEvt 6)
(defconst diskEvt 7)
(defconst activateEvt 8)
(defconst osEvt 15)

(defconst mDownMask 2)
(defconst mUpMask 4)
(defconst keyDownMask 8)
(defconst keyUpMask 16)
(defconst autoKeyMask 32)
(defconst updateMask 64)
(defconst diskMask 128)
(defconst activMask 256)
(defconst highLevelEventMask 1024)
(defconst osMask -32768)

(defconst activeFlag 1)
(defconst btnState 128)
(defconst cmdKey 256)
(defconst shiftKey 512)
(defconst alphaLock 1024)
(defconst optionKey 2048)
(defconst controlKey 4096)

(defconst charCodeMask 255)

; pascal Boolean WaitMouseUp(void)
;  = 0xA977; 
(deftrap WaitMouseUp ("a977")
  nil
  char)

; pascal void GetMouse(Point *mouseLoc)
;  = 0xA972; 
(deftrap GetMouse ("a972")
  ((mouseLoc address))
  nil)


;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defvar dialog-user-item-callback-proc-list nil)

(defun dialog-user-item-callback (d item)
  (let ((callback (assoc item dialog-user-item-callback-proc-list)))
    (if callback
		(condition-case errmsg
			(funcall (cdr callback) d item)
		  (error
		   nil)))))

; pascal DialogPtr GetNewDialog(short dialogID,void *dStorage,WindowPtr behind)
; = 0xA97C; 
(deftrap GetNewDialog ("a97c")
  ((dialogID short)
   (dStorage long)
   (behind long))
  long)

; pascal void ModalDialog(ModalFilterProcPtr filterProc,short *itemHit)
; = 0xA991; 
(deftrap ModalDialog-internal ("a991")
  ((filterProc long)
   (itemHit address))
  nil)

(setq modal-dialog-filter-proc nil)

(defun modal-dialog-filter-callback (d e i)
  (condition-case errmsg
	  (funcall modal-dialog-filter-proc d e i)
	(error
	 nil)))

(defun ModalDialog (filterProc itemHit)
  (if (and (integerp filterProc) (zerop filterProc))
	  (ModalDialog-internal 0 itemHit)
	(setq modal-dialog-filter-proc filterProc)
	(ModalDialog-internal modal-dialog-filter-callback itemHit)))

; pascal short Alert(short alertID,ModalFilterProcPtr filterProc)
;  = 0xA985; 
(deftrap Alert-internal ("a985")
  ((alertID short)
   (filterProc long))
  short)

(defun Alert (alertID filterProc)
  (if (and (integerp filterProc) (zerop filterProc))
	  (Alert-internal alertID 0)
	(setq modal-dialog-filter-proc filterProc)
	(Alert-internal alertID modal-dialog-filter-callback)))

; pascal short StopAlert(short alertID,ModalFilterProcPtr filterProc)
;  = 0xA986;
(deftrap StopAlert-internal ("a986")
  ((alertID short)
   (filterProc long))
  short)

(defun StopAlert (alertID filterProc)
  (if (and (integerp filterProc) (zerop filterProc))
	  (StopAlert-internal alertID 0)
	(setq modal-dialog-filter-proc filterProc)
	(StopAlert-internal alertID modal-dialog-filter-callback)))

; pascal short NoteAlert(short alertID,ModalFilterProcPtr filterProc)
;  = 0xA987;
(deftrap NoteAlert-internal ("a987")
  ((alertID short)
   (filterProc long))
  short)

(defun NoteAlert (alertID filterProc)
  (if (and (integerp filterProc) (zerop filterProc))
	  (NoteAlert-internal alertID 0)
	(setq modal-dialog-filter-proc filterProc)
	(NoteAlert-internal alertID modal-dialog-filter-callback)))

; pascal short CautionAlert(short alertID,ModalFilterProcPtr filterProc)
;  = 0xA988;
(deftrap CautionAlert-internal ("a988")
  ((alertID short)
   (filterProc long))
  short)

(defun CautionAlert (alertID filterProc)
  (if (and (integerp filterProc) (zerop filterProc))
	  (CautionAlert-internal alertID 0)
	(setq modal-dialog-filter-proc filterProc)
	(CautionAlert-internal alertID modal-dialog-filter-callback)))

; pascal void DisposeDialog(DialogPtr theDialog)
; = 0xA983; 
(deftrap DisposeDialog ("a983")
  ((theDialog long))
  nil)

;pascal void GetDItem(DialogPtr theDialog,short itemNo,short *itemType,Handle *item,
; Rect *box)
; = 0xA98D; 
(deftrap GetDItem ("a98d")
  ((theDialog long)
   (itemNo short)
   (itemType address)
   (item address)
   (box address))
  nil)

;pascal void SetDItem(DialogPtr theDialog,short itemNo,short itemType,Handle item,
; const Rect *box)
; = 0xA98E; 
(deftrap SetDItem ("a98e")
  ((theDialog long)
   (itemNo short)
   (itemType short)
   (item address)
   (box address))
  nil)

; pascal void SetIText(Handle item,ConstStr255Param text)
;  = 0xA98F; 
(deftrap SetIText ("a98f")
  ((item address)
   (text address))
  nil)

; pascal void GetIText(Handle item,Str255 text)
;  = 0xA990; 
(deftrap GetIText ("a990")
  ((item address)
   (text address))
  nil)

; pascal void SelIText(DialogPtr theDialog,short itemNo,short strtSel,short endSel)
;  = 0xA97E; 
(deftrap SelIText ("a97e")
  ((theDialog long)
   (itemNo short)
   (strtSel short)
   (endSel short))
  nil)

; pascal void ParamText(ConstStr255Param param0,ConstStr255Param param1,ConstStr255Param param2,
;  ConstStr255Param param3)
; = 0xA98B; 
(deftrap ParamText ("a98b")
  ((param0 address)
   (param1 address)
   (param2 address)
   (param3 address))
  nil)

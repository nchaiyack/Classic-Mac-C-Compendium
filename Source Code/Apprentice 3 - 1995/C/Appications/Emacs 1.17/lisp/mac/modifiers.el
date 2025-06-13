;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst metaKey 1)

(defconst modifiers:ok-button 1)
(defconst modifiers:cancel-button 2)
(defconst modifiers:revert-button 3)
(defconst modifiers:default-button 4)
(defconst modifiers:simple-meta-button 5)
(defconst modifiers:first-check-box 6)

;;; This list is in the order the items appear in the dialog.
(defvar modifiers:key-sets
  (list
   ;;; format is (index denoted typed-default typed-oam offset-in-pref)
   (list 0 shiftKey shiftKey shiftKey 0)
   (list 1 controlKey controlKey controlKey 4)
   (list 2 alphaLock alphaLock alphaLock 8)
   (list 3 cmdKey cmdKey cmdKey 12)
   (list 4 optionKey optionKey (+ optionKey cmdKey) 16)
   (list 5 (+ controlKey shiftKey) (+ controlKey shiftKey) (+ controlKey shiftKey) 44)
   (list 6 (+ controlKey optionKey) (+ controlKey optionKey) (+ controlKey optionKey cmdKey) 48)
   (list 7 (+ shiftKey optionKey) (+ shiftKey optionKey) (+ shiftKey optionKey cmdKey) 20)
   (list 8 (+ controlKey shiftKey optionKey) (+ controlKey shiftKey optionKey) (+ controlKey shiftKey optionKey cmdKey) 52)
   (list 9 (+ alphaLock optionKey) (+ alphaLock optionKey) (+ alphaLock optionKey cmdKey) 24)
   (list 10 (+ cmdKey optionKey) (+ cmdKey optionKey) (+ alphaLock cmdKey optionKey controlKey) 28)
   (list 11 metaKey 0 optionKey 32)
   (list 12 (+ shiftKey metaKey) 0 (+ shiftKey optionKey) 36)
   (list 13 (+ controlKey metaKey) 0 (+ controlKey optionKey) 40)))

(defun modifiers:use-defaults-or-oam (oam)
  (let* ((x (mapcar (function (lambda (x) (list (nth (if oam 3 2) x) (nth 1 x))))
					modifiers:key-sets))
		 (y (apply (function vector) (apply (function append) x))))
	(setq modifier-vector y)))

(defun modifiers:use-defaults () (modifiers:use-defaults-or-oam nil))
(defun modifiers:use-option-as-meta () (modifiers:use-defaults-or-oam t))

(defun modifiers:set-vector-from-dialog ()
  (setq modifier-vector
		(apply (function vector)
			   (apply (function append)
					  (nmapcar
					   (function (lambda (var-name ch-list)
								   (list (apply (function +)
												(nmapcar
												 (function (lambda (ch mask) (* (GetCtlValue ch) mask)))
												 ch-list
												 (list shiftKey controlKey alphaLock cmdKey optionKey)))
										 (nth 1 var-name))))
					   modifiers:key-sets
					   chandles))))
  (let ((h (NewHandle (* 4 (length modifiers:key-sets)))))
	(if (zerop (MemError))
		(progn
		  (HLock h)
		  (mapcar (function (lambda (var)
							   (encode-internal (deref h) (nth 4 var) 'long
												(aref modifier-vector (* 2 (nth 0 var))))))
				  modifiers:key-sets)
		  (HUnlock h)
		  (set-preference "DATA" 132 h)))))

(defun modifiers:initialize ()
  (modifiers:use-defaults)
  (let ((pref (get-preference "DATA" 132)))
	(if (>= pref 0)
		(progn
		  (HLock pref)
		  (mapcar (function
				   (lambda (var)
					 (let ((offset (nth 4 var)))
					   (aset modifier-vector (* 2 (nth 0 var))
							 (if (< offset (GetHandleSize pref))
								 (prog2
								  (HLock pref)
								  (extract-internal (deref pref) offset 'long)
								  (HUnlock pref))
							   0)))))
				  modifiers:key-sets)
		  (DisposHandle pref)))))

(defun modifiers:set-defaults ()
  (modifiers:set-boxes (mapcar (function (lambda (x) (nth 2 x))) modifiers:key-sets)))

(defun modifiers:set-simple-meta ()
  (modifiers:set-boxes (mapcar (function (lambda (x) (nth 3 x))) modifiers:key-sets)))

(defun modifiers:revert ()
  (modifiers:set-boxes (mapcar
						(function (lambda (x) (aref modifier-vector (* 2 (nth 0 x)))))
						modifiers:key-sets)))

(defun modifiers:set-boxes (masks)
  (nmapcar
   (function (lambda (ch-list mask)
			   (nmapcar
				(function (lambda (h x) (SetCtlValue h (if (zerop (logand mask x)) 0 1))))
				ch-list (list shiftKey controlKey alphaLock cmdKey optionKey))))
   chandles masks))

(defun modifiers:extract-chandles ()
  (let ((a modifiers:key-sets)
		(type (make-string (c:sizeof 'short) 0))
		(h (make-string (c:sizeof 'Handle) 0))
		(i modifiers:first-check-box))
	(setq chandles nil)
	(while a
	  (setq chandles (cons (mapcar (function (lambda (k)
											   (GetDItem d (+ i k) type h box)
											   (extract-internal h 0 'unsigned-long)))
								   '(0 1 2 3 4))
						   chandles))
	  (setq a (cdr a))
	  (setq i (+ i 5))))
  (setq chandles (nreverse chandles)))

(defun do-modifiers (menu item)
  (let ((d (GetNewDialog 131 0 -1))
		(type (make-string (c:sizeof 'short) 0))
		(h (make-string (c:sizeof 'Handle) 0))
		(box (make-rect))
		chandles item-str item-int)
	(unwind-protect
		(progn
		  (setq item-str (NewPtr 2))
		  (if (zerop (MemError))
			  (progn
				(modifiers:extract-chandles)
				(modifiers:revert)
				(ShowWindow d)
				(InitCursor)
				(encode-internal item-str 0 'short 0)
				(while (progn (setq item-int (extract-internal item-str 0 'short))
							  (and (not (= item-int modifiers:ok-button))
								   (not (= item-int modifiers:cancel-button))))
				  (cond
				   ((= item-int modifiers:revert-button)
					(modifiers:revert))
				   ((= item-int modifiers:default-button)
					(modifiers:set-defaults))
				   ((= item-int modifiers:simple-meta-button)
					(modifiers:set-simple-meta))
				   ((>= item-int modifiers:first-check-box)
					(GetDItem d item-int type h box)
					(let ((chandle (extract-internal h 0 'unsigned-long)))
					  (SetCtlValue chandle (if (zerop (GetCtlValue chandle)) 1 0)))))
				  
				  (ModalDialog (function modifiers:filter) item-str))
				
				(if (= item-int modifiers:ok-button)
					(modifiers:set-vector-from-dialog)))))
	  (DisposeDialog d))))

(defun modifiers:filter (d e i)
  (let ((what (c:slotref 'EventRecord e 'what)))
    (cond
	 ((= what updateEvt)
	  (SetPort d)
	  (TextFont geneva)
	  (TextSize 9)
	  0)
     ((= what keyDown)
      (let ((c (logand (c:slotref 'EventRecord e 'message) charCodeMask))
			(modifiers (c:slotref 'EventRecord e 'modifiers)))
		(if (or (= c (string-to-char "\r")) (= c 3))
			(progn
			  (encode-internal i 0 'short stacksize-ok-button)
			  (blink d stacksize-ok-button)
			  1)
		  (if (and (= c (string-to-char ".")) (not (zerop (logand modifiers cmdKey))))
			  (progn
				(encode-internal i 0 'short stacksize-cancel-button)
				(blink d stacksize-cancel-button)
				1)
			0))))
     (t
      0))))

;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;;;
;;; Clipboard support functions
;;;

(defvar clipboard-name "*clipboard*")
(defvar clipboard-previous-scrapcount -1)

(defun clipboard-current-scrapcount ()
  (extract-internal (InfoScrap) 8 'short))

(defun string-to-scrap (s)
  (ZeroScrap)
  (PutScrap (length s) "TEXT" s))

(defun scrap-to-string ()
  (let* ((h (NewHandle 0))
		 (offset (make-string 4 0))
		 (length (GetScrap h "TEXT" offset)))
    (if (< length 0)
		length
	  (prog2
	   (HLock h)
	   (extract-internal (deref h) 0 'string length)
	   (DisposHandle h)))))

(defun get-clipboard-buffer ()
  (let ((clipboard (get-buffer-create clipboard-name)))
    (set-buffer clipboard)
    (if (not buffer-read-only)
	(toggle-read-only))
    clipboard))

;;; Response to the Copy command
(defun copy-region-to-clipboard ()
  (let* ((old-buffer (current-buffer))
	 (s (buffer-substring (point) (if (mark) (mark) (point))))
	 (clipboard (get-clipboard-buffer)))
    (set-buffer clipboard)
    (toggle-read-only)
    (erase-buffer)
    (insert s)
    (subst-char-in-region (point-min) (point-max) 10 13 t)
    (string-to-scrap (buffer-string))
    (subst-char-in-region (point-min) (point-max) 13 10 t)
    (toggle-read-only)
    (set-buffer old-buffer)))

;;; The C code that handle activate events looks for this function by name
;;; to make sure the clipboard has the current contents of the Scrap.
(defun make-clipboard-current ()
  (let* ((old-buffer (current-buffer))
	 (clipboard (get-clipboard-buffer)))
    (if (not (= (clipboard-current-scrapcount) clipboard-previous-scrapcount))
	(let ((s (scrap-to-string)))
	  (if (stringp s)
	      (progn
		(set-buffer clipboard)
		(toggle-read-only)
		(erase-buffer)
		(insert s)
		(subst-char-in-region (point-min) (point-max) 13 10 t)
		(toggle-read-only)
		(setq clipboard-previous-scrapcount (clipboard-current-scrapcount))))))
    (set-buffer old-buffer)
    clipboard))

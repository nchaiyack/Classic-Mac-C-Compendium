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
;;; The following files are included in the dump.
;;;
;;; Any file that includes a call to AEInstallEventHandler or menu traps must not be
;;; included in a dump, but instead be executed at each startup.
;;;

(load "mac/ctypes")
(load "mac/utils")
(load "mac/headers")
(load "mac/traps")
(load "mac/Types")
(load "mac/Errors")
(load "mac/Memory")
(load "mac/Resources")
(load "mac/Fonts")
(load "mac/Quickdraw")
(load "mac/Events")
(load "mac/Windows")
(load "mac/Controls")
(load "mac/Dialogs")
(load "mac/Colors")
(load "mac/Menus")
(load "mac/Processes")
(load "mac/GestaltEqu")
(load "mac/Desk")
(load "mac/Scrap")
(load "mac/OSUtils")
(load "mac/Palettes")
(load "mac/AppleEvents")
(load "mac/AEObjects")
(load "mac/AERegistry")
(load "mac/AEPackObject")
(load "mac/clipboard")
(load "mac/mouse")
(load "mac/about")
(load "mac/stacksize")
(load "mac/modifiers")
(load "mac/color-edit")

(setq macintosh 'macintosh)
(setq msdos 'msdos)
(setq binary 'binary)
(setq unix 'unix)

(setq binary-suffixes '(".elc"))

(autoload 'ispell "ispell" nil t)
(autoload 'ispell-word "ispell" nil t)
(define-key esc-map "$" 'ispell-word)

(defvar available-menu-ID 128)

(defun get-unique-menu-ID ()
  (prog1
      available-menu-ID
    (setq available-menu-ID (1+ available-menu-ID))))

;;; We redefine this function to make it stationery-aware.

(defun find-file-noselect (filename &optional nowarn)
  "Read file FILENAME into a buffer and return the buffer.
If a buffer exists visiting FILENAME, return that one,
but verify that the file has not changed since visited or saved.
The buffer is not selected, just returned to the caller."
  (setq filename (expand-file-name filename))
  ;; Get rid of the prefixes added by the automounter.
  (if (and (string-match automount-dir-prefix filename)
	   (file-exists-p (file-name-directory
			   (substring filename (1- (match-end 0))))))
      (setq filename (substring filename (1- (match-end 0)))))
  (if (file-directory-p filename)
      (if find-file-run-dired
	  (dired-noselect filename)
	(error "%s is a directory." filename))
    (let ((buf (get-file-buffer filename))
	  error)
      (if buf
	  (or nowarn
	      (verify-visited-file-modtime buf)
	      (cond ((not (file-exists-p filename))
		     (error "File %s no longer exists!" filename))
		    ((yes-or-no-p
		      (if (buffer-modified-p buf)
			  "File has changed since last visited or saved.  Flush your changes? "
			"File has changed since last visited or saved.	Read from disk? "))
		     (save-excursion
		       (set-buffer buf)
		       (revert-buffer t t)))))
	(save-excursion
	      ;;; Added
	  (let* ((spec (make-string (c:sizeof 'FSSpec) 0))
		 (err (unix-filename-to-FSSpec filename spec))
		 (have-stationery (and (zerop err) (AE-is-stationery spec))))
	    
	    (setq buf (if have-stationery
			  (generate-new-buffer "untitled")
			(create-file-buffer filename)))
	    (set-buffer buf)
	    (erase-buffer)
	    (condition-case ()
		(insert-file-contents filename (not have-stationery))
	      (file-error
	       (setq error t)
	       ;; Run find-file-not-found-hooks until one returns non-nil.
	       (let ((hooks find-file-not-found-hooks))
		 (while (and hooks
			     (not (funcall (car hooks))))
		   (setq hooks (cdr hooks))))))
	    (setq default-directory (file-name-directory filename))
	    (funcall
	     (if have-stationery (function stationery-after-find-file)
	       (function after-find-file))
	     error (not nowarn)))))
      buf)))

(defun stationery-after-find-file (&optional error warn)
  (setq buffer-read-only nil)
  (if noninteractive
      nil
    (if auto-save-default
	(auto-save-mode t)))
  (set-buffer-modified-p nil)
  (mapcar 'funcall find-file-hooks))

(provide 'mac-dumptime)

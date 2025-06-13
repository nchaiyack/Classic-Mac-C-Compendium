;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1995 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;;;
;;; Code to install a menu used to send events to Codewarrior
;;;

(defvar cw:68K-first t)
(defvar cw:first-only nil)

(defun cw:do-menu-compile (menu item)
  (let ((filename (buffer-file-name)))
	(if (not filename)
		(message "Buffer must be associated with a file")
	  (let ((err (cw:do-compile filename)))
		(report-error-in-message-line err)))))

(defun cw:do-menu-check-syntax (menu item)
  (let ((filename (buffer-file-name)))
	(if (not filename)
		(message "Buffer must be associated with a file")
	  (let ((err (cw:do-check-syntax filename)))
		(report-error-in-message-line err)))))

(defun cw:do-menu-bring-up-to-date (menu item)
  (let ((err (cw:do-bring-up-to-date)))
	(report-error-in-message-line err)))

(defun cw:do-menu-reset-file-paths (menu item)
  (let ((err (cw:do-reset-file-paths)))
	(report-error-in-message-line err)))

(defun cw:do-menu-make (menu item)
  (let ((err (cw:do-make)))
	(report-error-in-message-line err)))

(defvar cw:enable-debugging nil)

(defun cw:do-enable-debugging (menu item)
  (setq cw:enable-debugging (not cw:enable-debugging))
  (CheckItem cw:project-menu 11 (if cw:enable-debugging 1 0)))

(defun cw:do-menu-run (menu item)
  (let ((err (cw:do-run cw:enable-debugging)))
	(report-error-in-message-line err)))

(defun cw:do-menu-launch (menu item)
  (let ((err (cw:do-launch)))
	(report-error-in-message-line err)))

(defun cw:do-menu-which-first (menu item)
  (setq cw:68K-first (= item 1)))

(defun cw:do-menu-use-first-only (menu item)
  (setq cw:first-only (not cw:first-only)))

(defvar cw:have-menus nil)

(if (not cw:have-menus)
    (progn
      (setq cw:choose-cw-menu-id (get-unique-menu-ID))
      (setq cw:choose-cw-menu (NewMenu cw:choose-cw-menu-id ""))
	  (AppendMenu cw:choose-cw-menu "Try 68K Compiler First" 'cw:do-menu-which-first)
	  (AppendMenu cw:choose-cw-menu "Try PPC Compiler First" 'cw:do-menu-which-first)
      (AppendMenu cw:choose-cw-menu "(-" nil)
	  (AppendMenu cw:choose-cw-menu "Use First Choice Only" 'cw:do-menu-use-first-only)
      (InsertMenu cw:choose-cw-menu -1)

      (setq cw:project-menu (NewMenu (get-unique-menu-ID) "Project"))
      (AppendMenu cw:project-menu "Choose CodeWarrior/\033" nil)
      (SetItemMark cw:project-menu 1 cw:choose-cw-menu-id)
      (AppendMenu cw:project-menu "Launch CodeWarrior/0" 'cw:do-menu-launch)
      (AppendMenu cw:project-menu "(-" nil)
      (AppendMenu cw:project-menu "Check Syntax/;" 'cw:do-menu-check-syntax)
      (AppendMenu cw:project-menu "Compile/K" 'cw:do-menu-compile)
      (AppendMenu cw:project-menu "(-" nil)
      (AppendMenu cw:project-menu "Reset File Paths" 'cw:do-menu-reset-file-paths)
      (AppendMenu cw:project-menu "Bring Up To Date/U" 'cw:do-menu-bring-up-to-date)
      (AppendMenu cw:project-menu "Make/M" 'cw:do-menu-make)
      (AppendMenu cw:project-menu "(-" nil)
      (AppendMenu cw:project-menu "Enable debugging" 'cw:do-enable-debugging)
      (AppendMenu cw:project-menu "Run/R" 'cw:do-menu-run)
      (InsertMenu cw:project-menu 0)

	  (DrawMenuBar)

      (setq cw:have-menus t)))

(defun cw:fixup-menus ()
  (CheckItem cw:choose-cw-menu 1 (if cw:68K-first 1 0))
  (CheckItem cw:choose-cw-menu 2 (if cw:68K-first 0 1))
  (CheckItem cw:choose-cw-menu 4 (if cw:first-only 1 0)))

(defvar MenuSelect-before-hooks nil)
(setq MenuSelect-before-hooks (cons (function cw:fixup-menus) MenuSelect-before-hooks))

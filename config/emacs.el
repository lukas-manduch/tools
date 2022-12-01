;; Added by Package.el.  This must come before configurations of
;; installed packages.  Don't delete this line.  If you don't want it,
;; just comment it out by adding a semicolon to the start of the line.
;; You may delete these explanatory comments.
(package-initialize)

(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(ansi-color-faces-vector
   [default default default italic underline success warning error])
 '(ansi-color-names-vector
   ["black" "#d55e00" "#009e73" "#f8ec59" "#0072b2" "#cc79a7" "#56b4e9" "white"])
 '(custom-enabled-themes (quote (deeper-blue)))
 '(dired-recursive-copies (quote always))
 '(dired-recursive-deletes (quote always))
 '(dired-x-hands-off-my-keys nil)
 '(global-visual-line-mode t)
 '(ibuffer-saved-filter-groups
   (quote
    (("Files & Folders"
      ("Folders"
       (used-mode . dired-mode))
      ("Files"
       (not used-mode . eshell-mode)
       (filename . "/"))))))
 '(ibuffer-saved-filters
   (quote
    (("gnus"
      ((or
        (mode . message-mode)
        (mode . mail-mode)
        (mode . gnus-group-mode)
        (mode . gnus-summary-mode)
        (mode . gnus-article-mode))))
     ("programming"
      ((or
        (mode . emacs-lisp-mode)
        (mode . cperl-mode)
        (mode . c-mode)
        (mode . java-mode)
        (mode . idl-mode)
        (mode . lisp-mode)))))))
 '(ido-enable-flex-matching t)
 '(ido-everywhere t)
 '(org-startup-indented t)
 '(package-selected-packages (quote (jedi nhexl-mode)))
 '(show-trailing-whitespace t))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;; WINDOWS SPECIFIC ;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(if(eq system-type 'windows-nt)
    (setenv "PATH" (concat "C:\\msys64\\usr\\bin;" (getenv "PATH") ))
  )
(if(eq system-type 'windows-nt)
    (setq exec-path (append '("C:/msys64/usr/bin")  exec-path))
  )
(if(eq system-type 'windows-nt)
    (w32-send-sys-command 61488)
  )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;; GNU/LINUX SPECIFIC ;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(if(eq system-type 'gnu/linux )
    (setq python-shell-interpreter "/bin/python3.6")
  )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(require 'ido)
(require 'ibuffer)
(require 'uniquify)
(require 'evil)

;; Always highlight parenthesis
(desktop-save-mode 1)
(electric-layout-mode 1)
(electric-pair-mode 1)
(global-hl-line-mode 1)
(ido-mode t)
(savehist-mode 1)
(show-paren-mode 1)
(transient-mark-mode 0) ; Don't show highlight


(setq column-number-mode t)
(setq desktop-restore-eager 5)
(setq next-line-add-newlines t) ; C-n will always add new line
(setq dired-dwim-target t) ; Dired guess target
(setq grep-find-use-xargs 'exec-plus) ; Execute greps in one process
(setq ido-use-filename-at-point 'guess)
(setq load-home-init-file t) ; don't load init file from ~/.xemacs/init.el
(setq visual-line-fringe-indicators '(left-curly-arrow right-curly-arrow)) ; Show arrows on long lines
(setq uniquify-buffer-name-style 'post-forward)
(setq evil-want-C-u-scroll t)
; Always split horizontally with ctrl-4-
(setq split-height-threshold nil)
(setq split-width-threshold 0)
(setq evil-default-state 'emacs)

;;;;

(setq-default indent-tabs-mode nil)


(put 'dired-do-copy 'ido 'find-file)
(put 'dired-do-rename 'ido 'find-file)
(put 'dired-find-alternate-file 'disabled nil)
(put 'downcase-region 'disabled nil)
(put 'narrow-to-region 'disabled nil)
(put 'upcase-region 'disabled nil)


(global-set-key (kbd "C-x C-b") 'ibuffer)
(global-set-key [f11] 'toggle-frame-fullscreen)
(global-set-key (kbd "M-o") 'other-window)
(define-key ibuffer-mode-map (kbd "M-o") 'other-window)

;; Always answer only y or n
(defalias 'yes-or-no-p 'y-or-n-p)


(add-to-list 'auto-mode-alist '("\\.m\\'" . octave-mode))
;; Open header files as c++
(add-to-list 'auto-mode-alist '("\\.h\\'" . c++-mode))
;; Save also register contents
(add-to-list 'desktop-globals-to-save 'register-alist)

;; HOOKS
(add-hook 'before-save-hook 'delete-trailing-whitespace)

(add-hook 'dired-load-hook
          (function (lambda () (load "dired-x"))))

(add-hook 'ibuffer-mode-hook
          (lambda ()
            (define-key ibuffer-mode-map "\C-x\C-f"
              'ibuffer-ido-find-file)))

(add-hook 'c-mode-common-hook
	  (lambda () (setq indent-tabs-mode t)
            (c-set-offset 'substatement-open 0)
            (c-set-offset 'brace-list-open 0)
            (hs-minor-mode 1)
            )
          )

(add-hook 'c++-mode-common-hook
	  (lambda () (setq indent-tabs-mode t)
            (c-set-offset 'substatement-open 0)
            (c-set-offset 'brace-list-open 0)
            (hs-minor-mode 1)
            )
          )

(with-eval-after-load 'evil
    (defalias #'forward-evil-word #'forward-evil-symbol))

;; SMART TABS
(load-file "~/.emacs.d/smart_tabs_mode.el")
(smart-tabs-insinuate 'c 'c++ )


; Show file full path in title bar
(setq-default frame-title-format
   (list '((buffer-file-name " %f"
             (dired-directory
              dired-directory
              (revert-buffer-function " %b"
                                      ("%b - Dir:  " default-directory)))))))
; Disable toolbars
(progn
  (if (fboundp 'tool-bar-mode) (tool-bar-mode -1))
  (menu-bar-mode -1)
  (scroll-bar-mode -1)
  )



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;; ORG - MODE ;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(setq org-src-fontify-natively t)
(global-set-key "\C-cl" 'org-store-link)
(global-set-key "\C-ca" 'org-agenda)
(global-set-key "\C-cc" 'org-capture)
(global-set-key "\C-cb" 'org-iswitchb)

(setq org-directory "~/notes")
(setq org-default-notes-file (concat org-directory "/quick_notes.org"))
(setq org-agenda-files (list "~/notes"))

(setq org-mobile-inbox-for-pull "~/notes/inbox.org")
(setq org-mobile-directory "~/MobileOrg")
(setq org-mobile-files '("~/notes/"))

(require 'org-crypt)
(org-crypt-use-before-save-magic)
(setq org-tags-exclude-from-inheritance (quote ("crypt")))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;; FUNCTIONS ;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun ido-goto-symbol (&optional symbol-list)
      "Refresh imenu and jump to a place in the buffer using Ido."
      (interactive)
      (unless (featurep 'imenu)
        (require 'imenu nil t))
      (cond
       ((not symbol-list)
        (let ((ido-mode ido-mode)
              (ido-enable-flex-matching
               (if (boundp 'ido-enable-flex-matching)
                   ido-enable-flex-matching t))
              name-and-pos symbol-names position)
          (unless ido-mode
            (ido-mode 1)
            (setq ido-enable-flex-matching t))
          (while (progn
                   (imenu--cleanup)
                   (setq imenu--index-alist nil)
                   (ido-goto-symbol (imenu--make-index-alist))
                   (setq selected-symbol
                         (ido-completing-read "Symbol? " symbol-names))
                   (string= (car imenu--rescan-item) selected-symbol)))
          (unless (and (boundp 'mark-active) mark-active)
            (push-mark nil t nil))
          (setq position (cdr (assoc selected-symbol name-and-pos)))
          (cond
           ((overlayp position)
            (goto-char (overlay-start position)))
           (t
            (goto-char position)))))
       ((listp symbol-list)
        (dolist (symbol symbol-list)
          (let (name position)
            (cond
             ((and (listp symbol) (imenu--subalist-p symbol))
              (ido-goto-symbol symbol))
             ((listp symbol)
              (setq name (car symbol))
              (setq position (cdr symbol)))
             ((stringp symbol)
              (setq name symbol)
              (setq position
                    (get-text-property 1 'org-imenu-marker symbol))))
            (unless (or (null position) (null name)
                        (string= (car imenu--rescan-item) name))
              (add-to-list 'symbol-names name)
              (add-to-list 'name-and-pos (cons name position))))))))

(global-set-key (kbd "M-i") 'ido-goto-symbol) ; or any key you see fit



;;;;;;;;;;;;;;;;;;;;;
(defun ibuffer-ediff-marked-buffers ()
  (interactive)
  (let* ((marked-buffers (ibuffer-get-marked-buffers))
         (len (length marked-buffers)))
    (unless (= 2 len)
      (error (format "%s buffer%s been marked (needs to be 2)"
                     len (if (= len 1) " has" "s have"))))
    (ediff-buffers (car marked-buffers) (cadr marked-buffers))))

(define-key ibuffer-mode-map "e" 'ibuffer-ediff-marked-buffers)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun ibuffer-ido-find-file (file &optional wildcards)
  "Like `ido-find-file', but default to the directory of the buffer at point."
  (interactive
   (let ((default-directory
           (let ((buf (ibuffer-current-buffer)))
             (if (buffer-live-p buf)
                 (with-current-buffer buf
                   default-directory)
               default-directory))))
     (list (ido-read-file-name "Find file: " default-directory) t)))
  (find-file file wildcards))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun dired-ediff-marked-files ()
    "Run ediff on marked ediff files."
    (interactive)
    (set 'marked-files (dired-get-marked-files))
    (when (= (safe-length marked-files) 2)
        (ediff-files (nth 0 marked-files) (nth 1 marked-files)))

    (when (= (safe-length marked-files) 3)
        (ediff3 (buffer-file-name (nth 0 marked-files))
                (buffer-file-name (nth 1 marked-files))
                (buffer-file-name (nth 2 marked-files)))))

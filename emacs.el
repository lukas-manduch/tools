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
 '(custom-enabled-themes (quote (wombat)))
  '(dired-x-hands-off-my-keys nil)
 '(dired-recursive-copies (quote always))
 '(dired-recursive-deletes (quote always))
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
 '(package-selected-packages (quote (nhexl-mode))))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )
(if(eq system-type 'windows-nt)
    (
     (setenv "PATH" (concat "C:\\msys64\\usr\\bin;" (getenv "PATH") ))
     (setq exec-path (append '("C:/msys64/usr/bin")  exec-path))
     )
 (put 'dired-find-alternate-file 'disabled nil)

;; Always highlight parenthesis
(show-paren-mode 1)
(setq column-number-mode t)
(transient-mark-mode 0) ; Don't show highlight

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
(defun my-c-mode-hook ()
  ; (setq auto-fill-function 'do-auto-fill)
  ;(c-set-style "Linux")
  ;(c-set-offset 'substatement-open '0) ; brackets should be at same indentation level as the statements they open
  ;(c-set-offset 'inline-open '+)
  ;(c-set-offset 'block-open '+)
  ;(c-set-offset 'brace-list-open '+)   ; all "opens" should be indented by the c-indent-level
  ;(c-set-offset 'case-label '+)
  )       ; indent case labels by c-indent-level, too

(setq-default indent-tabs-mode nil)

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

(load-file "~/.emacs.d/smart-tabs-mode.el")
(smart-tabs-insinuate 'c 'c++ )

(setq python-shell-interpreter "/bin/python3.6")



; Show file full path in title bar
(setq-default frame-title-format
   (list '((buffer-file-name " %f"
             (dired-directory
              dired-directory
              (revert-buffer-function " %b"
                                      ("%b - Dir:  " default-directory)))))))
(progn
  (if (fboundp 'tool-bar-mode) (tool-bar-mode -1))
  (menu-bar-mode -1)
  (scroll-bar-mode -1)
)
(add-hook 'before-save-hook 'delete-trailing-whitespace)

(require 'ido)
(ido-mode t)
(setq ido-use-filename-at-point 'guess)

(setq load-home-init-file t) ; don't load init file from ~/.xemacs/init.el

(add-hook 'dired-load-hook
          (function (lambda () (load "dired-x"))))

;; Open header files as c++
(add-to-list 'auto-mode-alist '("\\.h\\'" . c++-mode))
(put 'upcase-region 'disabled nil)
(put 'downcase-region 'disabled nil)

(setq grep-find-use-xargs 'exec-plus)

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

(require 'ibuffer)
(add-hook 'ibuffer-mode-hook
          (lambda ()
            (define-key ibuffer-mode-map "\C-x\C-f"
              'ibuffer-ido-find-file)))

(global-set-key (kbd "C-x C-b") 'ibuffer)
(global-set-key [f11] 'toggle-frame-fullscreen)
(global-set-key (kbd "M-o") 'other-window)

(define-key ibuffer-mode-map (kbd "M-o") 'other-window)

(savehist-mode 1)                       ; Save minibuffer
(electric-layout-mode 1)
(electric-pair-mode 1)
(hl-line-mode 1)


(w32-send-sys-command 61488)

(desktop-save-mode 1)
(setq desktop-restore-eager 5)

;; C-n will always add new line
(setq next-line-add-newlines t)
(setq dired-dwim-target t)

(setq dired-dwim-target t)              ;

(put 'narrow-to-region 'disabled nil)
(put 'dired-do-rename 'ido 'find-file)
(put 'dired-do-copy 'ido 'find-file)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;; ORG - MODE ;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(setq org-src-fontify-natively t)
(require 'org-annotate-file)
;;;;;;;;;;;



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

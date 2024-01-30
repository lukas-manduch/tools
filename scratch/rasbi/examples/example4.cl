;; This is complete example of common lisp program, that is executed same with
;; both rasbi and sbcl interpreter.

(let ((filename "README.md"))
  (if (probe-file filename)
    (progn
      (format t "File ~a exists ~%" filename)
      (format t "This is sooo ~a~%" 1337)
      )))

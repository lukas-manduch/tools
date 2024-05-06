(let (( i 0 )) 
  (tagbody 
    someth
    (setq i (+ i 1))
    (progn
    (format t "Huha ~a~%" i)
    (format t "What a line~%")
    )
    (if (< i 5)
      (goto someth)
      )
    end
    )
  )

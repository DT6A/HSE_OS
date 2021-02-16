  .global write_to_file
  .extern strlen                 # needed to write  

  .text

write_to_file:                   # bool write_to_file(const char *path, const char *text);
  pushq %rsi                     # store text
  pushq %rsi                     # need it twice

  movq $2, %rax                  # open system call
  movq $01101, %rsi              # flags create if needed (00100) + truncate (01000) + write (00001)
  movq $0644, %rdx               # mode
 
  syscall                        # open file
 
  cmp $0, %rax                   # check if failed
  jle fail_ret

  popq %rdi                      # restore text
  pushq %rax                     # store file descriptor
  call strlen                    # get string length

  movq %rax, %rdx                # string length to write
  movq $1, %rax                  # write system call
  popq %rdi                      # file descriptor to write
  popq %rsi                      # string to write
  
  pushq %rdi                     # store file descriptor
  pushq %rdx                     # store length

  syscall                        # do write

  popq %rdx                      # restore length
  movq $1, %rcx                  # successfull termination
  cmp %rax, %rdx                 # compare length and number of bytes writen
  je succ_ret                    # if same finish with success
  movq $0, %rcx                  # unsuccessfull termnination
  jmp succ_ret
    

fail_ret:                        ### return 0 as result
  popq %rdi                      # restore text
  popq %rdi                      # restore text
  movq $0, %rax
  ret

succ_ret:                        ### close file and return 1 as result
  movq $3, %rax                  # close system call
  popq %rdi                      # get file descriptor
  pushq %rcx                     # store result
  sub $8, %rsp                   # move stack
  syscall
  add $8, %rsp                   # restore stack

  popq %rax                      # return result
  ret

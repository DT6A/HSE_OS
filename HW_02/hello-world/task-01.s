  .global print_hello
  .extern fputs, stdout

  .section .rodata
.Lstr_start:
  .asciz "Welcome, "
.Lstr_end:
  .asciz ", to the assembly world!"

  .text
print_hello:                   # void print_hello(const char *name);
  push %rdi

  sub $8, %rsp
  movq stdout, %rsi
  movq $.Lstr_start, %rdi
  call fputs
  add $8, %rsp

  pop %rdi
  movq stdout, %rsi
  call fputs

  movq $.Lstr_end, %rdi
  movq stdout, %rsi
  call fputs

  ret

  .global product_of_min_and_max_primes, is_prime, aprox_int_sqrt

  .text

aprox_int_sqrt:
  push %rbx
  mov $0, %eax

sqloop:
  push %rax
  movq %rax, %rbx
  mulq %rbx
  movq %rax, %rbx
  pop %rax
  cmp %rdi, %rbx
  jge sqend
  add $1, %eax
  jmp sqloop

sqend:
  pop %rbx
  ret


is_prime:                                        # uint32_t is_prime(uint32_t);
  push %rbx
  push %rdx

  cmp $0, %rdi                                   # Check 0, 1, 2
  je nprime
  cmp $1, %rdi
  je nprime
  cmp $2, %rdi
  je prime

  push %rdi
  call aprox_int_sqrt 
  movq %rax, %rbx
  add $1, %rbx
  pop %rdi

ploop:
  sub $1, %rbx
  cmp $1, %rbx
  je prime
  movq %rdi, %rax
  xor %rdx, %rdx
  div %rbx
  cmp $0, %rdx
  je nprime
  jmp ploop

prime:
  mov $1, %eax
  jmp pret

nprime:
  mov $0, %eax
  jmp pret

pret:
  pop %rdx
  pop %rbx
  ret


product_of_min_and_max_primes:                   # uint64_t product_of_min_and_max_primes(const uint32_t *data, uint32_t size);
  push %rbx
  push %rcx
  push %r8

  movq $0, %rbx
  mov $4294967295, %ecx

  movq %rdi, %r8
  movq $0, %rdi

mloop:
  cmp $0, %rsi
  je mret

  mov (%r8), %edi
  call is_prime
  cmp $0, %eax
  je mloope

ma:
  cmp %rdi, %rbx
  jge mi
  movq %rdi, %rbx
mi:
  cmp %rdi, %rcx
  jle mloope
  movq %rdi, %rcx

mloope:
  sub $1, %rsi
  add $4, %r8
  jmp mloop

mret:
  movq %rcx, %rax
  mulq %rbx

  pop %r8
  pop %rcx
  pop %rbx
  ret

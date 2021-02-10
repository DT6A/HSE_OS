  .global product_of_min_and_max_primes, is_prime, aprox_int_sqrt

  .text

aprox_int_sqrt:                                  # uint32_t
  mov $0, %eax

sqloop:
  pushq %rax
  movq %rax, %rdx
  mulq %rdx
  movq %rax, %rdx
  popq %rax
  cmp %rdi, %rdx
  jge sqend
  add $1, %eax
  jmp sqloop

sqend:
  ret


is_prime:                                        # uint32_t is_prime(uint32_t);  
  cmp $0, %rdi                                   # Check 0, 1, 2
  je nprime
  cmp $1, %rdi
  je nprime
  cmp $2, %rdi
  je prime

  call aprox_int_sqrt 
  movq %rax, %r8
  add $1, %r8

ploop:
  sub $1, %r8
  cmp $1, %r8
  je prime
  movq %rdi, %rax
  xor %rdx, %rdx
  div %r8
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
  ret


product_of_min_and_max_primes:                   # uint64_t product_of_min_and_max_primes(const uint32_t *data, uint32_t size);
  pushq %rbx                                     # stack align -8
  
  movq $0, %rbx
  mov $4294967295, %ecx

  movq %rdi, %r8
  movq $0, %rdi

mloop:
  cmp $0, %rsi
  je mret

  mov (%r8), %edi
  pushq %r8                                      # stack align -16, fine
  call is_prime
  popq %r8
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

  popq %rbx
  ret

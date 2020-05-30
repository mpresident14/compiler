.text
.align 16

.section .data
__heaptop:
  .quad 0
__mallocaddr:
  .quad 0

.section .text
__findheap:
  movq $12, %rax            # syscall 12
  xorq %rdi, %rdi           # set addr to nullptr so that it returns the
  syscall                   #     current top of the heap
  movq %rax, __mallocaddr(%rip)
  movq %rax, __heaptop(%rip)
  retq


__brk:
  movq $12, %rax
  addq __heaptop(%rip), %rdi  # extend the heap by some amount
  syscall
  movq %rax, __heaptop(%rip)
  retq


.globl __malloc
__malloc:
  pushq %r12
  pushq %r13
  movq __mallocaddr(%rip), %r12
  movq __heaptop(%rip), %rcx
  movq %rdi, %r13
  # mallocAddr + numBytes
  movq %r12, %rax
  addq %r13, %rax
  # #if (mallocaddr + numBytes > __heaptop)
  cmpq %rax, %rcx
  jle LDONE
  # TODO: Should allocate min(numBytes, 4096)
  # heaptop = brk(4096)
  movq $4096, %rdi
  callq __brk
  movq %rax, __heaptop(%rip)
LDONE:
  # int ret = mallocaddr
  movq %r12, %rax
  # mallocaddr = mallocaddr + numBytes
  addq %r13, %r12
  movq %r12, __mallocaddr(%rip)
  popq %r13
  popq %r12
  retq

# print:
#   movq $1, %rax
#   movq $1, %rdi
#   syscall
#   retq

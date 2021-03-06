.text
.align 16

.section .data
__heaptop:
  .quad 0
__mallocaddr:
  .quad 0

.section .text

# Note: %rcx, %r11, and %rax are volatile across syscall

.globl _start
_start:
  callq __findheap
  callq main
  callq __exit

__exit:
  movq $60, %rax  # syscall 60
  movq $0, %rdi   # error code 0
  syscall

# Volatile: %rdi, %rcx, %r11, %rax
__findheap:
  movq $12, %rax            # syscall 12
  xorq %rdi, %rdi           # set addr to nullptr so that it returns the
  syscall                   #     current top of the heap
  movq %rax, __mallocaddr(%rip)
  movq %rax, __heaptop(%rip)
  retq


# Volatile: %rdi, %rcx, %r11, %rax
__brk:
  movq $12, %rax
  addq __heaptop(%rip), %rdi  # extend the heap by some amount
  syscall
  movq %rax, __heaptop(%rip)
  retq


__malloc:
  pushq %r12
  pushq %r13
  movq __mallocaddr(%rip), %r12
  movq %rdi, %r13
  # mallocAddr + numBytes
  leaq (%r12, %r13), %rax
  # #if (mallocaddr + numBytes > __heaptop)
  cmpq __heaptop(%rip), %rax
  jbe LDONE
  # heaptop = brk(max(numBytes, 4096))
  cmpq $4096, %rdi
  jg LCONT
L4096:
  movq $4096, %rdi
LCONT:
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

__print:          # print(char[], int)
  movq $1, %rax   # syscall 1 (write(int fd, char* msg, size_t nbytes))
  movq %rsi, %rdx # nbytes
  movq %rdi, %rsi # msg
  movq $1, %rdi   # Write to stdout: fd = 1
  syscall
  retq

__println:
  callq __print
  subq $8, %rsp      # print newline at the end
  movq $1, %rax
  movq $1, %rdi
  movq $10, (%rsp)   # '\n' = 10
  leaq (%rsp), %rsi
  movq $1, %rdx
  syscall
  addq $8, %rsp
  retq

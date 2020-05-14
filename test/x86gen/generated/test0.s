.text
.globl runprez
.align 16
runprez:
	subq $0, %rsp
	movq $1, %rax
	movq $5, %rcx
	leaq (%rax, %rcx, 2), %rax
	movq %rax, %rcx
	cmpq $11, %rcx
	je EQ
	movq $-1, %rdi
	jmp FIN
EQ:
	movq %rcx, %rdi
FIN:
	callq printInt
	addq $0, %rsp
	retq

.text
.align 16
String_0:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	movq $24, %rdi
	callq __malloc
	movq %rax, %rbx
	movq $0, %r12
	movq %r12, %rdi
	addq $8, %rdi
	callq __malloc
	movq %r12, (%rax)
	movq %rax, 8(%rbx)
	movq $0, 16(%rbx)
	movq %rbx, %rax
	popq %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
String_1:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r13
	pushq %r12
	movq %rdi, %r12
	movq %rsi, %rbx
	movq $24, %rdi
	callq __malloc
	movq %rax, %r13
	cmpq $0, %rbx
	je _M1
_M0:
	movq %r12, 8(%r13)
	mov (%r12), %rax
	movq %rax, 16(%r13)
	jmp _M2
_M1:
	mov (%r12), %rbx
	movq %rbx, %rdi
	addq $8, %rdi
	callq __malloc
	movq %rbx, (%rax)
	movq %rax, 8(%r13)
	movq $0, %rcx
	cmpq %rbx, %rcx
	jge _M4
_M3:
	movsbq 8(%r12, %rcx, 1), %rax
	mov 8(%r13), %rdx
	movb %al, 8(%rdx, %rcx, 1)
	incq %rcx
	cmpq %rbx, %rcx
	jl _M3
_M4:
	movq %rbx, 16(%r13)
_M2:
	movq %r13, %rax
	popq %r12
	popq %r13
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
String_append_2:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	movq %rdi, %r12
	movq %rsi, %rbx
	mov 16(%rbx), %rcx
	mov 8(%rbx), %rax
	mov (%rax), %rax
	cmpq %rcx, %rax
	jne _M6
_M5:
	movq %rbx, %rdi
	callq String_resize_4
	jmp _M7
_M6:
_M7:
	mov 8(%rbx), %rdi
	leaq 16(%rbx), %rax
	mov (%rax), %rdx
	leaq (%rax), %rax
	mov (%rax), %rcx
	incq %rcx
	movq %rcx, (%rax)
	movb %r12b, 8(%rdi, %rdx, 1)
	movq %rbx, %rax
	popq %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
String_reverse_3:
	pushq %rbp
	movq %rsp, %rbp
	movq $0, %rdx
	mov 16(%rdi), %rax
	decq %rax
	cmpq %rax, %rdx
	jge _M9
_M8:
	mov 8(%rdi), %rcx
	movsbq 8(%rcx, %rdx, 1), %rsi
	mov 8(%rdi), %rcx
	movsbq 8(%rcx, %rax, 1), %rcx
	mov 8(%rdi), %r8
	movb %cl, 8(%r8, %rdx, 1)
	mov 8(%rdi), %rcx
	movb %sil, 8(%rcx, %rax, 1)
	incq %rdx
	decq %rax
	cmpq %rax, %rdx
	jl _M8
_M9:
	movq %rbp, %rsp
	popq %rbp
	retq
String_resize_4:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	movq %rdi, %r12
	movq $1, %rax
	mov 8(%r12), %rcx
	mov (%rcx), %rcx
	leaq (%rax, %rcx, 2), %rbx
	movq %rbx, %rdi
	addq $8, %rdi
	callq __malloc
	movq %rbx, (%rax)
	movq $0, %rcx
	mov 16(%r12), %rdx
	cmpq %rdx, %rcx
	jge _M11
_M10:
	mov 8(%r12), %rdx
	movsbq 8(%rdx, %rcx, 1), %rdx
	movb %dl, 8(%rax, %rcx, 1)
	incq %rcx
	mov 16(%r12), %rdx
	cmpq %rdx, %rcx
	jl _M10
_M11:
	movq %rax, 8(%r12)
	popq %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
toString_5:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r13
	pushq %r12
	movq %rdi, %rbx
	callq String_0
	movq %rax, %r12
	cmpq $0, %rbx
	jne _M13
_M12:
	movq $48, %rdi
	movq %r12, %rsi
	callq String_append_2
	popq %r12
	popq %r13
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
	jmp _M14
_M13:
_M14:
	movq $0, %r13
	cmpq $0, %rbx
	jge _M16
_M15:
	negq %rbx
	movq $1, %r13
	jmp _M17
_M16:
_M17:
	cmpq $0, %rbx
	je _M19
_M18:
	movq %rbx, %rax
	movq $10, %rcx
	cqto
	idivq %rcx
	addq $48, %rdx
	movsbq %dl, %rdi
	movq %r12, %rsi
	callq String_append_2
	movq %rbx, %rax
	movq $10, %rcx
	cqto
	idivq %rcx
	movq %rax, %rbx
	cmpq $0, %rbx
	jne _M18
_M19:
	cmpq $0, %r13
	je _M21
_M20:
	movq $45, %rdi
	movq %r12, %rsi
	callq String_append_2
	jmp _M22
_M21:
_M22:
	movq %r12, %rdi
	callq String_reverse_3
	movq %r12, %rax
	popq %r12
	popq %r13
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
toString_6:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	cmpq $0, %rdi
	je _M24
_M23:
	movq $4, %rbx
	movq %rbx, %rdi
	addq $8, %rdi
	callq __malloc
	movq %rbx, (%rax)
	movb $116, 8(%rax)
	movb $114, 9(%rax)
	movb $117, 10(%rax)
	movb $101, 11(%rax)
	movq $1, %rsi
	movq %rax, %rdi
	callq String_1
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
	jmp _M25
_M24:
	movq $5, %rbx
	movq %rbx, %rdi
	addq $8, %rdi
	callq __malloc
	movq %rbx, (%rax)
	movb $102, 8(%rax)
	movb $97, 9(%rax)
	movb $108, 10(%rax)
	movb $115, 11(%rax)
	movb $101, 12(%rax)
	movq $0, %rsi
	movq %rax, %rdi
	callq String_1
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq
_M25:
toString_7:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	movq %rdi, %rbx
	callq String_0
	movq %rbx, %rdi
	movq %rax, %rsi
	callq String_append_2
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	retq

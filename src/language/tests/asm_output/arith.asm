.text
.align 16
main:
	pushq %rbp
	movq %rsp, %rbp
	movq $1, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $2, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $3, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $4, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $5, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $6, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $7, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $8, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $9, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $10, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $11, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $12, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $13, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $14, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $15, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $16, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq $4, %rcx
	movq %rcx, %rax
	negq %rax
	imulq %rcx, %rax
	negq %rax
	incq %rax
	movq %rax, %rdi
	callq toString_7
	mov 8(%rax), %rdi
	addq $8, %rdi
	mov 16(%rax), %rsi
	callq __println
	movq %rbp, %rsp
	popq %rbp
	retq

.text
.globl _masking
_masking:

		pushq		%rbp
		pushq		%rbx
		pushq		%r8
		pushq		%r9
		pushq		%r10
		pushq		%r13
		pushq		%r14
		pushq		%r15

		movq    	$0x2492492492492492, %rbx	## mask of 3
		movq    	$0x4210842108421084, %r8	## mask of 5
		movq    	$0x810204081020408, %r9		## mask of 7
		movq    	$0x1002004008010020, %r10	## mask of 11
		movq    	$0x400200100080040, %r13	## mask of 13
		movq    	$0x800040002000100, %r14	## mask of 17
		movq    	$0x800010000200, %r15		## mask of 19

		# last elem of the list
		shlq		$3, %rsi
		addq		%rdi, %rsi			## %rsi = 8 * %rsi + %rdi

		xor			%rbp, %rbp 					## accumulator of the masks
		orq			%rbx, %rbp
		orq			%r8, %rbp
		orq			%r9, %rbp
		orq			%r10, %rbp
		orq			%r13, %rbp
		orq			%r14, %rbp
		orq			%r15, %rbp

		movq		%rbp, (%rdi)

		# stepping to the next item of the array
		addq		$8, %rdi

masking_L1:
		xor			%rbp, %rbp 					## accumulator of the masks

# for 3
		movq    	%rbx, %rax
		shrq		$61, %rax
		shrdq		$1, %rax, %rbx
		orq			%rbx, %rbp

# for 5
		movq    	%r8, %rax
		shlq		$59, %rax
		shldq		$1, %rax, %r8
		orq			%r8, %rbp

# for 7
		movq    	%r9, %rax
		shrq		$57, %rax
		shrdq		$1, %rax, %r9
		orq			%r9, %rbp

# for 11
		movq    	%r10, %rax
		shlq		$53, %rax
		shldq		$2, %rax, %r10
		orq			%r10, %rbp

# for 13
		movq    	%r13, %rax
		shlq		$51, %rax
		shldq		$1, %rax, %r13
		orq			%r13, %rbp

# for 17
		movq    	%r14, %rax
		shlq		$47, %rax
		shldq		$4, %rax, %r14
		orq			%r14, %rbp

# for 19
		movq    	%r15, %rax
		shrq		$45, %rax
		shrdq		$7, %rax, %r15
		orq			%r15, %rbp

		movq		%rbp, (%rdi)

		# stepping to the next item of the array
		addq		$8, %rdi
		cmpq		%rdi, %rsi
		jg			masking_L1




		popq		%r15
		popq		%r14
		popq		%r13
		popq		%r10
		popq		%r9
		popq		%r8
		popq		%rbx
		popq		%rbp

        ret


.globl _testing							## testing(%rdi, %rsi)
_testing:
		shlq		$3, %rsi
		addq		%rdi, %rsi			## %rsi = 8 * %rsi + %rdi

		xor			%rax, %rax
testing_L1:
		addq		(%rdi), %rax

		addq		$8, %rdi
		cmpq		%rdi, %rsi
		jg			testing_L1

		ret

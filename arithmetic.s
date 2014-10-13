#
#   Copyright 2014 by Janos Kasza
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

.text
# Usage:
# uint64_t asm_masking(uint64_t *ptr, unsigned length, unsigned table_offset);
#                                %rdi          %rsi             %edx
.align 4,0x90
.globl _asm_masking
_asm_masking:
		# %r10 and %r12 are still free
		pushq		%rbp
		pushq		%rbx
		pushq		%r8
		pushq		%r9
		pushq		%r13
		pushq		%r14
		pushq		%r15

		movq    	$0x349249649a4924b2, %rbx	## mask of 3 and 11
		movq    	$0x4a10a4618942148c, %r8	## mask of 5 and 7
		movq    	$0x400200100080040, %r13	## mask of 13
		movq    	$0x800040002000100, %r14	## mask of 17
		movq    	$0x800010000200, %r15		## mask of 19

		testl		%edx, %edx					## don't skip at all if table_offset == 0
		je			masking_L16

		movl		%edx, %r9d					## save the table_offset

# skipping for 3 and 11
		movl		%r9d, %eax					## prepare edx:eax for the division
		movl		$33, %ecx
		xorl		%edx, %edx
		divl		%ecx						## %edx = table_offset % 33
		testl		%edx, %edx
		je			masking_L3
masking_L2:
		shlq		$2, %rbx
		movq		%rbx, %rax
		shrq		$33, %rax
		orq			%rax, %rbx

		decl		%edx
		testl		%edx, %edx
		jne			masking_L2
masking_L3:

# skipping for 5 and 7
		movl		%r9d, %eax					## prepare edx:eax for the division
		movl		$35, %ecx
		xorl		%edx, %edx
		divl		%ecx						## %edx = table_offset % 35
		testl		%edx, %edx
		je			masking_L5
masking_L4:
		shlq		$6, %r8
		movq		%r8, %rax
		shrq		$35, %rax
		orq			%rax, %r8

		decl		%edx
		testl		%edx, %edx
		jne			masking_L4
masking_L5:

# skipping for 13
		movl		%r9d, %eax					## prepare edx:eax for the division
		movl		$13, %ecx
		xorl		%edx, %edx
		divl		%ecx						## %edx = table_offset % 13
		testl		%edx, %edx
		je			masking_L11
masking_L10:
		shlq		$1, %r13
		movq		%r13, %rax
		shrq		$13, %rax
		orq			%rax, %r13

		decl		%edx
		testl		%edx, %edx
		jne			masking_L10
masking_L11:

# skipping for 17
		movl		%r9d, %eax					## prepare edx:eax for the division
		movl		$17, %ecx
		xorl		%edx, %edx
		divl		%ecx						## %edx = table_offset % 17
		testl		%edx, %edx
		je			masking_L13
masking_L12:
		shlq		$4, %r14
		movq		%r14, %rax
		shrq		$17, %rax
		orq			%rax, %r14

		decl		%edx
		testl		%edx, %edx
		jne			masking_L12
masking_L13:

# skipping for 19
		movl		%r9d, %eax					## prepare edx:eax for the division
		movl		$19, %ecx
		xorl		%edx, %edx
		divl		%ecx						## %edx = table_offset % 19
		testl		%edx, %edx
		je			masking_L15
masking_L14:
		shrq		$7, %r15
		movq		%r15, %rax
		shlq		$19, %rax
		orq			%rax, %r15

		decl		%edx
		testl		%edx, %edx
		jne			masking_L14
masking_L15:

		movl		%r9d, %edx					## saving back the table_offset, maybe needed later

# start the masking
masking_L16:									## after the skip
		# last item of the list
		shlq		$3, %rsi
		addq		%rdi, %rsi					## %rsi = 8 * %rsi + %rdi

		xor			%rbp, %rbp 					## accumulator of the masks
		orq			%rbx, %rbp
		orq			%r8, %rbp
		orq			%r13, %rbp
		orq			%r14, %rbp
		orq			%r15, %rbp

		movq		%rbp, (%rdi)


		# stepping to the _second_ item of the array
		addq		$8, %rdi

# loop of the masking
masking_L1:
		xor			%rbp, %rbp 					## accumulator of the masks

# for 3 and 11
		shlq		$2, %rbx
		movq		%rbx, %rax
		shrq		$33, %rax
		orq			%rax, %rbx
		orq			%rbx, %rbp

# for 5 and 7
		shlq		$6, %r8
		movq		%r8, %rax
		shrq		$35, %rax
		orq			%rax, %r8
		orq			%r8, %rbp

# for 13
		shlq		$1, %r13
		movq		%r13, %rax
		shrq		$13, %rax
		orq			%rax, %r13
		orq			%r13, %rbp

# for 17
		shlq		$4, %r14
		movq		%r14, %rax
		shrq		$17, %rax
		orq			%rax, %r14
		orq			%r14, %rbp

# for 19
		shrq		$7, %r15
		movq		%r15, %rax
		shlq		$19, %rax
		orq			%rax, %r15
		orq			%r15, %rbp

# setting ptr[i] to the accumulated mask
 		movq		%rbp, (%rdi)

		# stepping to the next item of the array
		addq		$8, %rdi
		cmpq		%rdi, %rsi
		jg			masking_L1



		# exiting
		popq		%r15
		popq		%r14
		popq		%r13
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

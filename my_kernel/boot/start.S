#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>




.section .text.excVec
NESTED(exceptVec, 0, sp)
		.set	noat
		.set	noreorder
1:	
		nop
		mfc0	k1,CP0_CAUSE
		
		la		k0,my_exception_handlers
		andi	k1,0x7c		
		addu	k0,k1
		lw		k0,(k0)		
		# NOP
		nop
		jr	k0
		nop
END(exceptVec)
		
		
		
		.set	at
		.data
			.globl mCONTEXT
mCONTEXT:
			.word 0 

			.global	KERNEL_SP;
KERNEL_SP:
    		.word		0

			.globl msg
msg:
			.word 0

			.globl USTACKTOP
USTACKTOP:
			.word 0



	
	.text
LEAF(my_kernel_entry)                      
	
	.set	mips2      
	.set	reorder

	
	mtc0	zero, CP0_STATUS

	
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

	
	mfc0	t0, CP0_CONFIG
	and		t0, ~0x7
	ori		t0, 0x2
	mtc0	t0, CP0_CONFIG


	li		sp, 0x80400000

	li		t0,0x80400000
	sw		t0,mCONTEXT

	jal main
	nop
		
loop:
	j	loop
	nop

END(my_kernel_entry)           

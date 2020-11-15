#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>
#include <myClock.h>


//设置时钟频率并且开中断

	.text
LEAF(Clock_set)
	nop
	li t0, 0x11
	sb t0, 0xb5000100
	mfc0 t0, CP0_STATUS
	or t0, 0x10001001|0

	xor t0, 0
	mtc0 t0, CP0_STATUS
	sw sp, KERNEL_SP
	jr ra
	nop
END(Clock_set)

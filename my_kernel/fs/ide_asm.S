#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>



LEAF(read_sector)
	sw  a0, 0x93000010 	
	sw	a1, 0x93000000  
	li	t0, 0
	sb	t0, 0x93000020 
	lw  v0, 0x93000030
	nop
	jr	ra
	nop
END(read_sector)


LEAF(write_sector)
	sw a0,0x93000010
    sw a1,0x93000000
	li t0,1
    sb t0,0x93000020
	lw v0,0x93000030
	nop
	jr ra
	nop
END(write_sector)

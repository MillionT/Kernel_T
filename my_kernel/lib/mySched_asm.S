#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>
#include <mySched.h>

.set noreorder
LEAF(spTransfer)

    lw sp, USTACKTOP
    nop
	jr	ra
	nop
END(spTransfer) 

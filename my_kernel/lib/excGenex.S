#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>
#include <myTrapStack.h>


.macro  CREATE_HANDLER exception handler clear
    .align	5
	NESTED(handle_\exception, TF_SIZE, sp)  
	.set	noat
	nop
	SAVE_CPU				
	\clear
	.set	at
	move	a0, sp	
	jal	    \handler	
	nop
	j	ret_from_exception
	nop
	END(handle_\exception)
.endm

LEAF(do_reserved)
END(do_reserved)


//关中断
CREATE_HANDLER reserved do_reserved CLOSE_INT


//从异常中返回
FEXPORT(ret_from_exception)
	.set noat
	.set noreorder
	RESTORE_CPU
	.set at  
	nop
	lw	k0,TF_EPC(sp)
	mtc0 k0,CP0_EPC
	sw  k0,msg
	# jal sched_print
	//回到最开始的栈顶
	lw	sp,TF_REG29(sp) 

	li 		v0,0x10001001
	mtc0 	v0, CP0_STATUS

	nop
	nop
	nop
	jr	k0					 
	
	# eret



.set noreorder
.align	5
NESTED(handle_int, TF_SIZE, sp)

    # mfc0  k0, CP0_STATUS
    # sw  k0,msg
    # jal sched_print
    .set	noat
    nop

    SAVE_CPU
    CLOSE_INT

    .set	at
    mfc0	v0,CP0_EPC
    sw		v0,TF_EPC(sp)
    # sw		v0,msg  

    lw		sp,TF_REG29(sp)	//将sp保持不变


    mfc0	t0, CP0_CAUSE
    mfc0	t2, CP0_STATUS
    and		t0, t2

    andi	t1, t0, STATUSF_IP4
    bnez	t1, sched_handler
    nop
END(handle_int)



sched_handler:

1:
    jal	sched_yield	
	nop
	j	ret_from_exception
	nop









#include <myPrintf.h>
#include <asm/asm.h>
#include <myPages.h>
#include <myProcess.h>
#include <myClock.h>
#include <myTrap.h>
#include <myConsole.h>
#include <myFs.h>


int main()
{
	myPrintf("系统开始启动\n\n");



	myPrintf("\n********************************************\n");
	myPrintf("  __          _ _ _ _ _    ____________\  
\n |  |        |   _ _ _  | |   _____S___|\ 
\n |  |        |  |  0  | | |_________ |\    
\n |  |______  |  |_____| |  ________| |\ 
\n |_________| |__________| |__________|\n");
	myPrintf("\n********************************************\n\n");


	mmu_init();
	myPrintf("\n\n\n");
	process_setup();

	
	init_fs();
	read_from_block(1, f);
    read_from_block(2, &bitMapBlock);
    fCount = super.root->FC;

	myPrintf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
	myPrintf("完成进程管理机制的初始化\n");
	myPrintf("异常机制和时钟中断机制开始启动!\n");
	myPrintf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	trap_init();
    Clock_setup();

	// while(1);
	// kernel_panic(__FILE__, __LINE__);

	return 0;
}



unsigned long to_p_addr(unsigned long v_addr)
{
	if(v_addr < K_START)
		kernel_panic(__FILE__, __LINE__);
	return (v_addr - K_START);
}



unsigned long to_v_addr(unsigned long p_addr)
{
	unsigned long pn = (p_addr >> 12);
	if(pn >= pagesNum)
		kernel_panic(__FILE__, __LINE__);
	return (p_addr + K_START);
}




#include <mySched.h>
#include <myProcess.h>
#include <myPages.h>
#include <myConsole.h>
#include <myClock.h>

extern unsigned long KERNEL_SP;
extern unsigned long msg;
extern void spTransfer();
extern int USTACKTOP;


void sched_print()
{
    // myPrintf("sizeof(unsigned long) = %d", sizeof(unsigned long)); 
    myPrintf("msg = %#08x\n", msg);
    kernel_panic(__FILE__, __LINE__);
    return;
}


void sched_yield()
{
    int max = 0;
    struct Process *iterator = processRunnableListHead->nxtProcess;
    struct Process *maxP = NULL;
    while(iterator != NULL)
    {
        if((max < iterator->priority) && (iterator->status == process_status_running))
        {
            max = iterator->priority;
            maxP = iterator;
        }
        iterator = iterator->nxtProcess;
    }
    if(maxP != NULL)
    {
        struct Process *pt = maxP->preProcess;
        pt->nxtProcess = maxP->nxtProcess;
        if(maxP->nxtProcess != NULL)
        {
            maxP->nxtProcess->preProcess = pt;
        }
        else
        {
            processRunnableListTail = pt; 
        } 
        getProcessStackTop(maxP); 
        spTransfer();
        //此时已经将sp转到分配的物理页位置，所以执行下面的函数时会把返回地址放在分配的物理页用户栈上
        run_process(maxP);
        //但是返回后用的栈实际上还是用户栈，所以内核还是存在一些问题
    }
    if(processRunnableListHead == processRunnableListTail)
    {
        console_init();
        return;
    }   
    return;
}
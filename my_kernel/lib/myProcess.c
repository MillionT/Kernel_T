#include <myMemory.h>
#include <myTypes.h>
#include <myPrintf.h>
#include <myProcess.h>
#include <myPages.h>
#include <myTrap.h>

extern struct Process *myProcesses = NULL;
extern struct Process *curProcess = NULL;


extern unsigned long *setUpPgTableLevel1_dir; //内核一级页表
extern char *KERNEL_SP;
extern int USTACKTOP;


//为每一个进程生成id
unsigned int create_id(struct Process *p)
{
    static unsigned long for_nxt_id = 0;
    for_nxt_id++;//从1开始
    unsigned int rid = (for_nxt_id << 11);
    return rid;
}


struct Process pt;
struct Process pt_runnable;
//初始化空闲进程链表，可运行进程链表
void process_setup()
{
    // struct Process p;
    struct Process *ptmp;
    
    processListHead = &pt;
    // myPrintf("processListHead = %x\n", processListHead);
    processListHead->nxtProcess = NULL;
    processListHead->preProcess = NULL;
    processListTail = processListHead;


    processRunnableListHead = &pt_runnable;
    // myPrintf("processRunnableListHead = %x\n", processRunnableListHead);
    processRunnableListHead->nxtProcess = NULL;
    processRunnableListHead->preProcess = NULL;
    processRunnableListTail = processRunnableListHead;

    int j;
    //插入到空闲进程管理链表中，倒序插入
    for(j = processesNums - 1; j >= 0; j--)
    {
        ptmp = myProcesses + j;
        ptmp->status = process_status_free;
        processListTail->nxtProcess = ptmp;
        ptmp->preProcess = processListTail;
        processListTail = ptmp;
        ptmp->nxtProcess = NULL;
    }
    // myPrintf("processListTail = %x\n", processListTail);
    // myPrintf("myProcess[0].status = %d, &myProcess[0] = %x\n", myProcesses[0].status, &myProcesses[0]);
}

//初始化当前进程的进程页表
int process_mapping(struct Process *p)
{
    struct Page *page = NULL;
    int f;
    if((f = free_page_alloc(&page)) != 0)
    {
        kernel_panic(__FILE__, __LINE__);
        return -1;
    }
    page->count++;
    unsigned long *pgTableLevel1_dir_t = to_v_addr((page - myPages) * PAGESIZE);

    int i;
    for(i = 0; i < ((U_MY_PROCESSES >> 22) & 0x03ff); i++)
        pgTableLevel1_dir_t[i] = 0;
    
  
    for(i = ((U_MY_PROCESSES >> 22) & 0x03ff); i <= (((~0) >> 22) & 0x03ff); i++)
    {
        pgTableLevel1_dir_t[i] = setUpPgTableLevel1_dir[i];
    }
    p->pgTableLevel1_dir = pgTableLevel1_dir_t;
    p->pgTableLevel1_dir_p = ((page - myPages) * PAGESIZE);

    p->pgTableLevel1_dir[(( K_START + BLOCKSIZE)>> 22 & 0x03ff)] = p->pgTableLevel1_dir_p | valid_bit | read_bit;
    p->pgTableLevel1_dir[(( K_START - BLOCKSIZE)>> 22 & 0x03ff)] = p->pgTableLevel1_dir_p | valid_bit | read_bit;
    return 0;
}

//从空闲链表中获取一个进程
int process_alloc(struct Process **p, unsigned int parent_id)
{
    if((processListHead == processListTail))
    {
        kernel_panic(__FILE__, __LINE__);
        return -1;
    }
    //取出队列尾部的一个进程
    struct Process *tmp;
    struct Process *tmp2;
    tmp = processListTail;
    tmp2 = processListTail->preProcess;
    processListTail = tmp2;
    processListTail->nxtProcess = NULL;

    process_mapping(tmp);
    tmp->p_id = parent_id;
    tmp->status = process_status_running;
    //myPrintf("here!\n");
    tmp->id = create_id(tmp);
    tmp->tf.cp0_status = 0x10001001;
    //设置用户栈指针
    tmp->tf.regs[29] = U_MY_PROCESSES - 2 * 4 * 1024;
    *p = tmp;
    // myPrintf("process_alloc()成功！\n");
    return 0;
}


void process_add_to_runnable_list(struct Process *p)
{
    processRunnableListTail->nxtProcess = p;
    p->preProcess = processRunnableListTail;
    processRunnableListTail = p;
    p->nxtProcess = NULL;
}

void process_new(int cPriority, struct Process **ret)
{
    struct Process *p;
    //父进程都是0
    if(process_alloc(&p, 0) < 0)
    {
        myPrintf("error here to new a process!\n");
        return;
    }
    p->priority = cPriority;
    //用户栈
    struct Page *page = NULL;
    if(free_page_alloc(&page) != 0)
    {
        kernel_panic(__FILE__, __LINE__);
        return;
    }

    unsigned long permission;
    permission = valid_bit | read_bit;

    page_map_running(p->pgTableLevel1_dir, permission, U_STACK_TOP - 4 * 1024, page);
    
    //再分配一页作为进程的资源
    if(free_page_alloc(&page) < 0)
    {
        kernel_panic(__FILE__, __LINE__);
        return -1;
    }

    page_map_running(p->pgTableLevel1_dir, permission, U_STACK_TOP - 2 * 4 * 1024, page);
    // process_add_to_runnable_list(p);
    myPrintf("进程p->id = %d 生成成功！\n", p->id >> 11);
    *ret = p;
}

//将页表清0
void process_add_to_free_list(struct Process *p)
{
    unsigned int p_addr;
    unsigned int i, j;
    

    for(i = 0; i < (U_MY_PROCESSES >> 22 & 0x03ff); i++)
    {
        if(p->pgTableLevel1_dir[i] & valid_bit)
        {
            // myPrintf("i =  %#08x\n", i);
            p_addr = ((unsigned long)(p->pgTableLevel1_dir[i]) & ~0xfff);
            // myPrintf("here! %#08x\n", p_addr);
            unsigned long *v_addr = (unsigned long*)to_v_addr(p_addr);
            for(j = 0; j <= (((unsigned long)(~0) >> 12) & (0x03ff)); j++)
            {
                //如果二级页表项是有效的
                if(v_addr[i] & valid_bit)
                {
                    
                    unmap(p->pgTableLevel1_dir, (i << 22) | (j << 12));
                }
            }
            p->pgTableLevel1_dir[i] = 0;
            
            struct  Page *page = myPages + ((unsigned long)p_addr >> 12);
            page->count--;
            if(page->count == 0)
            {
                page_add_to_free_list(page);
            }
        }
    }
    
    p_addr = p->pgTableLevel1_dir_p;
    p->pgTableLevel1_dir = 0;
    p->pgTableLevel1_dir_p = 0;
    
    struct  Page *page2 = myPages + ((unsigned long)p_addr >> 12);
    page2->count--;
    if(page2->count == 0)
    {
        page_add_to_free_list(page2);
    }
    
    p->status = process_status_free;
    //插入到空闲进程链表队首
    p->preProcess = processListHead;
    p->nxtProcess = processListHead->nxtProcess;
    processListHead->nxtProcess->preProcess = p;
    processListHead->nxtProcess = p;

}


//获得进程用户栈栈顶的物理地址，并赋值给USTACKTOP
void getProcessStackTop(struct Process *p)
{
    unsigned long processStackTop = v_addr2p_addr(p->pgTableLevel1_dir, (U_STACK_TOP - 4 * 1024));
    if(processStackTop != (~0))
    {
        processStackTop += 4 * 1024;
        USTACKTOP = processStackTop;
    }
    else
    {
        myPrintf("进程用户栈映射出错!\n");
        kernel_panic(__FILE__, __LINE__);
    }
    
}

void run_process(struct Process *p)
{

    int tst = 10;
    // myPrintf("&tst = %#08x\n", &tst);
    //实际上可以让进城控制块中包含这个结构，就像linux进程的tss一样
    struct Trapframe *preProcessTrapframe = (struct Trapframe*)(MY_TIMESTACK - sizeof(struct Trapframe));
    if(curProcess)
    {
        copy_from_src(preProcessTrapframe, &(curProcess->tf), sizeof(struct Trapframe));
    }
    
    curProcess = p;
    int i = (int)(p->id >> 11);
    if(pRunCount[i] == 0)
    {
        myPrintf("\n进程 %d 开始执行，优先级别为 %d\n", p->id >> 11, p->priority);
    }
    myPrintf("进程 %d 第 %d 次运行！\n", p->id >> 11, ++pRunCount[i]);
    if(pRunCount[i] >= 5)
    {
        
        process_add_to_free_list(curProcess);
        myPrintf("\n进程 %d 执行结束\n\n", i);
        pRunCount[i] = 0;
        return;
    }
    //重新加入到可运行进程链表中
    process_add_to_runnable_list(p);
    //测试当前链表中共有多少个进程
    struct Process *it = processRunnableListHead->nxtProcess;
    int cnt = 0;
    while(it)
    {
        cnt++;
        it = it->nxtProcess;
    }
    // myPrintf("cnt = %d\n", cnt);
    return;
}



#ifndef _MY_PROCESS_H
#define _MY_PROCESS_H

#include "myTypes.h"
#include "myMemory.h"
#include "myTrap.h"

//设置最多1024个进程
#define processesNums (1 << 10) 
#define U_MY_PROCESSES (0x80000000 - (3 * 4 * 1024 * 1024))
#define U_STACK_TOP (U_MY_PROCESSES)

#define process_status_free 0
#define process_status_blocked 1
#define process_status_running 2

struct Process{
    struct Trapframe tf; 
    unsigned int id;
    unsigned int p_id; //父进程id
    unsigned int status; //运行状态
    int priority;//优先级
    unsigned long *pgTableLevel1_dir;//一级页表的虚拟地址
    unsigned int pgTableLevel1_dir_p;//一级页表物理地址
    struct Process *preProcess;
    struct Process *nxtProcess;
};

//空闲进程链表
struct Process *processListHead;
struct Process *processListTail;

//可运行进程链表
struct Process *processRunnableListHead;
struct Process *processRunnableListTail;

struct Process *myProcesses;
struct Process *curProcess;

unsigned int create_id(struct Process *p);
void process_setup();
int process_mapping(struct Process *p);
int process_alloc(struct Process **p, unsigned int parent_id);
void process_new(int cPriority, struct Process **ret);
void process_add_to_free_list(struct Process *p);
void run_process(struct Process *p);
void process_add_to_runnable_list(struct Process *p);
void getProcessStackTop(struct Process *p);

static int pRunCount[1024] = {0};

#endif
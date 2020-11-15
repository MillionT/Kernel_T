#include <myConsole.h>
#include <myPrintf.h>
#include <asm/asm.h>
#include <myPages.h>
#include <myProcess.h>
#include <myClock.h>
#include <myTrap.h>
#include <mySched.h>
#include <myFs.h>


extern char getChar();
int runCount = 0;
extern int fCount = 3;


void console_init()
{
    while(1)
    {
        myPrintf("[Kernel_T@localhost] $ ");
        char ch;
        consoleCount = 0;
        while((ch = getChar()) != '\r')
        {
            //退格键
            if(ch == 0x08)
            {
                if(consoleCount > 0)
                {
                    consoleBuf[--consoleCount] = 0;
                }
                continue;
            }
            consoleBuf[consoleCount++] = ch;
            myPrintf("%c", ch);
        }
        consoleBuf[consoleCount] = 0;
        myPrintf("\n");
        int flag = console_read();
        if(flag == 1)
            break;
    }
}


int console_read()
{
    
    int consoleBufLen = consoleCount;
    char console_args[10][30];
    int bufP = 0;
    int argP = 0;
    while(consoleBuf[bufP] != ' ')
    {
        console_args[0][argP++] = consoleBuf[bufP];
        bufP++;
    }
    console_args[0][argP] = 0;

    if(strcmp(console_args[0], "echo") == 0)
    {
        int i;
        while(consoleBuf[bufP] == ' ')
            bufP++;
        for(i = bufP; i < consoleBufLen; i++)
            myPrintf("%c", consoleBuf[i]);
        myPrintf("\n\n");
    }
    else if(strcmp(console_args[0], "test") == 0)
    {
        while(consoleBuf[bufP] == ' ')
            bufP++;
        
        argP = 0;
        while(consoleBuf[bufP] != 0)
        {
            console_args[1][argP++] = consoleBuf[bufP];
            bufP++;
        }
        console_args[1][argP] = 0;
        if(strcmp(console_args[1], "printf") == 0)
        {
            myPrintfTest();
            myPrintf("\n");
        }
        else if(strcmp(console_args[1], "mmu") == 0)
        {
            page_function_test();
            page_init_after_test();
            myPrintf("\n");
        }
        else if(strcmp(console_args[1], "process") == 0)
        {
            myPrintf("process test!\n\n");
            // process_setup();
            // myPrintf("process_setup()执行成功！\n");
            struct Process *p1, *p2, *p3, *p4, *p5;
            process_new(3, &p1);
            process_new(2, &p2);
            process_new(1, &p3);
            process_new(4, &p4);
            process_new(5, &p5);
            process_add_to_runnable_list(p1);
            process_add_to_runnable_list(p2);
            process_add_to_runnable_list(p3);
            process_add_to_runnable_list(p4);
            process_add_to_runnable_list(p5);
            //创建5个进程
            myPrintf("\n所有进程创建成功!\n");
            return 1;
            // Clock_setup();
        }
        else
        {
            myPrintf("  test后的指令不匹配，请从下列指令中重新选择: \n");
            myPrintf("     1. test printf\n");
            myPrintf("     2. test mmu\n");
            myPrintf("     3. test process\n\n");
        }
        
    }
    else if(strcmp(console_args[0], "new") == 0)
    {
        myPrintf("  正在执行创建进程指令，请为进程指定一个权重(1-100):  ");
        char ch;
        int processPriority = 0;
        while((ch = getChar()) != '\r')
        {
            if(ch >= '0' && ch <= '9')
            {
                myPrintf("%c", ch);
                processPriority = 10 * processPriority + ch - '0';
            }
            else
            {
                myPrintf("错误的权重!\n");
                kernel_panic(__FILE__, __LINE__);
            }
        }
        myPrintf("\n");
        struct Process *pNew;
        process_new(processPriority, &pNew);
        runProcess[runCount++] = pNew;
        myPrintf("  进程生成成功!新进程的 id 为 %d, 权重为: %d\n\n", pNew->id >> 11, pNew->priority);
    }
    else if(strcmp(console_args[0], "show") == 0)
    {
        while(consoleBuf[bufP] == ' ')
            bufP++;
       
        argP = 0;
        while(consoleBuf[bufP] != 0)
        {
            console_args[1][argP++] = consoleBuf[bufP];
            bufP++;
        }
        console_args[1][argP] = 0;
        if(strcmp(console_args[1], "all") == 0)
        {
            if(runCount == 0)
            {
                myPrintf("  对不起，当前无可运行进程\n\n");
                return;
            }
            else
            {          
                myPrintf("  当前所有可运行进程为: \n");
                int h;
                for(h = 0; h < runCount; h++)
                {
                    myPrintf("      进程 %d \n", runProcess[h]->id >> 11);
                }
                myPrintf("\n");
            }
        }
        else
        {
            myPrintf("  请使用show all指令显示当前所有可运行进程\n");
        }     
    }
    else if(strcmp(console_args[0], "run") == 0)
    {
        while(consoleBuf[bufP] == ' ')
            bufP++;
        
        argP = 0;
        while(consoleBuf[bufP] != 0)
        {
            console_args[1][argP++] = consoleBuf[bufP];
            bufP++;
        }
        console_args[1][argP] = 0;
        if(strcmp(console_args[1], "all") == 0)
        {
            if(runCount == 0)
            {
                myPrintf("  没有可以运行的进程\n\n");
                return;
            }
            int h;
            for(h = 0; h < runCount; h++)
            {
                process_add_to_runnable_list(runProcess[h]);
            }
            runCount = 0;
            myPrintf("\n");
            return 1;
        }
        else
        {
            myPrintf("请使用run all指令运行进程\n");
        }     
    }
    else if(strcmp(console_args[0], "mkdir") == 0)
    {
        myPrintf("  正在执行创建目录指令，请输入新创建的目录名:  ");
        char cBuf[50];
        int cBufCount = 0;
        char ch2;
        while((ch2 = getChar()) != '\r')
        {
            //退格键
            if(ch2 == 0x08)
            {
                if(cBufCount > 0)
                {
                    cBuf[--cBufCount] = 0;
                }
                continue;
            }
            cBuf[cBufCount++] = ch2;
            myPrintf("%c", ch2);
        }
        cBuf[cBufCount] = 0;
        // myPrintf("  目录生成成功!新目录名为：%s\n\n", cBuf);
        int pos = fCount;
        fCount += 1;
        f[pos].fDir = curDir;
        int h;
        for(h = 0; h < 10; h++)
        {
            f[pos].directP[h] = 0;
        }
        strcpy(f[pos].name, cBuf);
        f[pos].type = 10;//表示为目录
        f[pos].directCount = 0;
        int i = curDir->directCount;
       
        curDir->directP[i] = (unsigned long)(&(f[pos]));
        curDir->directCount++;
        // myPrintf("%d\n",curDir->directCount);
        if(i >= 20)
            kernel_panic(__FILE__, __LINE__);
        myPrintf("\n  目录生成成功!新目录名为：%s\n\n", cBuf);  
        write_to_block(2, &bitMapBlock);
        write_to_block(1, f);     
    }
    else if(strcmp(console_args[0], "create") == 0)
    {
        myPrintf("  正在执行创建文件指令，请输入新创建的文件名:  ");
        char cBuf[50];
        int cBufCount = 0;
        char ch2;
        while((ch2 = getChar()) != '\r')
        {
            //退格键
            if(ch2 == 0x08)
            {
                if(cBufCount > 0)
                {
                    cBuf[--cBufCount] = 0;
                }
                continue;
            }
            cBuf[cBufCount++] = ch2;
            myPrintf("%c", ch2);
        }
        cBuf[cBufCount] = 0;
        myPrintf("\n  文件生成成功!新文件名为：%s\n\n", cBuf);
        int pos = fCount;
        // myPrintf("fCount = %d\n", fCount);
        fCount += 1;
        f[pos].fDir = curDir;
        strcpy(f[pos].name, cBuf);
        struct Page *p;
        free_page_alloc(&p);
        int cBlockNo = alloc_block_num();
        // myPrintf("分配的BlockNo = %d\n", cBlockNo);
        f[pos].directP[0] = cBlockNo;
        f[pos].type = 20;//表示为文件
        f[pos].directCount = 1;//表示被占用
        f[pos].size = 0;//以B为单位
        int h;
        for(h = 1; h < 10; h++)
        {
            f[pos].directP[h] = 0;
        }
        int i = curDir->directCount;
        curDir->directP[i] = (unsigned long)(&(f[pos]));
        curDir->directCount++;
        if(i >= 20)
            kernel_panic(__FILE__, __LINE__);
        
        myPrintf("  请向文件输入内容：\n\n");
        char inBuf[100];
        int inBufCount = 0;
        char ch3;
        while((ch3 = getChar()) != '\r')
        {
            //退格键
            if(ch3 == 0x08)
            {
                if(inBufCount > 0)
                {
                    inBuf[--inBufCount] = 0;
                }
                continue;
            }
            inBuf[inBufCount++] = ch3;
            myPrintf("%c", ch3);
        }
        inBuf[inBufCount] = 0;
        f[pos].size = inBufCount;
        void *p_addr = (p - myPages) << 12;
        for(i = 0; i <= inBufCount; i++)
        {
            *(char *)p_addr = inBuf[i];
            p_addr += 1;
        }
        // p_addr = (p - myPages) << 12;
        // for(i = 0; i <= inBufCount; i++)
        // {
        //     myPrintf("%c", *(char *)p_addr);
        //     p_addr+=1
        //     myPrintf("\n");
        // }
        unsigned long p_add = (p - myPages) << 12;
        write_to_block(cBlockNo, p_add);
        page_add_to_free_list(p);
        super.root->FC = fCount;
        write_to_block(2, &bitMapBlock);
        write_to_block(1, f);
        myPrintf("\n\n  文件写入成功!\n\n");
    }
    else if(strcmp(console_args[0], "ls") == 0)
    {
        myPrintf("  当前目录下的文件有：\n");
        int j;
        // myPrintf("%d\n", curDir->directCount);
        for(j = 0; j < curDir->directCount; j++)
        {
            if(((struct File*)curDir->directP[j]) != 0)
                myPrintf("      %s\n", ((struct File*)curDir->directP[j])->name);
        }
        myPrintf("\n");
    }
    else if(strcmp(console_args[0], "pwd") == 0)
    {
        myPrintf("  当前目录为：\n");
        myPrintf("      %s\n", curDir->name);
        myPrintf("\n");
    }
    else if(strcmp(console_args[0], "bit") == 0)
    {
        struct Page *p;
        free_page_alloc(&p);
        unsigned long p_addr = (p - myPages) << 12;
        read_from_block(2, p_addr);
        void *pp_addr = (p - myPages) << 12;
        int t = 0;
        while((*(int *)pp_addr) != 0)
        {
            if(t == 0)
                myPrintf("%32b\n", (*(int *)pp_addr));
            pp_addr += 4;
            t++;
        }
        myPrintf("t = %d\n", t);//共32个int
        page_add_to_free_list(p);
    }
    else if(strcmp(console_args[0], "init") == 0)
    {
        read_from_block(1, f);
        read_from_block(2, &bitMapBlock);
        fCount = super.root->FC;
        // myPrintf("super.root->name = %s\n", super.root->name);
        // myPrintf("super.root->FC = %d\n", super.root->FC);
    }
    else if(strcmp(console_args[0], "cd") == 0)
    {
       while(consoleBuf[bufP] == ' ')
            bufP++;
        
        argP = 0;
        while(consoleBuf[bufP] != 0)
        {
            console_args[1][argP++] = consoleBuf[bufP];
            bufP++;
        }
        console_args[1][argP] = 0;
        if(strcmp(console_args[1], "..") == 0)
        {
            if(curDir == super.root)
            {
                myPrintf("   当前位于根目录下！\n\n");
                return 0;
            }
            curDir = curDir->fDir;
        }
        else
        {
            int k;
            for(k = 0; k < 10; k++)
            {
                if(((struct File*)curDir->directP[k]) != 0)
                {
                    if(strcmp(console_args[1], ((struct File*)curDir->directP[k])->name) == 0)
                    {
                        curDir = ((struct File*)curDir->directP[k]);
                        break;
                    }
                }
            }
            if(k >= 10)
            {
                myPrintf("  输入路径错误！\n\n");
            }
        } 
    }
    else if(strcmp(console_args[0], "cat") == 0)
    {
       while(consoleBuf[bufP] == ' ')
            bufP++;
        
        argP = 0;
        while(consoleBuf[bufP] != 0)
        {
            console_args[1][argP++] = consoleBuf[bufP];
            bufP++;
        }
        console_args[1][argP] = 0;
       
        int k;
        for(k = 0; k < 10; k++)
        {
            if(((struct File*)curDir->directP[k]) != 0)
            {
                if(strcmp(console_args[1], ((struct File*)curDir->directP[k])->name) == 0)
                {
                    struct File *f1 = (struct File*)curDir->directP[k];
                    if(f1->type == 10)
                    {
                        myPrintf("错误！%s：是一个目录！\n", f1->name);
                        return 0;
                    }
                    // myPrintf("size = %d\n", f1->size);
                    int no = f1->directP[0];
                    struct Page *p;
                    free_page_alloc(&p);
                    unsigned long p_addr = (p - myPages) << 12;
                    read_from_block(no, p_addr);
                    void *pp_addr = (p - myPages) << 12;
                    myPrintf("\n");
                    while((*(char *)pp_addr) != 0)
                    {
                        myPrintf("%c", (*(char *)pp_addr));
                        pp_addr += 1;
                    }
                    myPrintf("\n\n");
                    page_add_to_free_list(p);
                    break;
                }
            }
        }
        if(k >= 10)
        {
            myPrintf("  %s: 文件不存在！\n\n", console_args[1]);
        }
         
    }
    else if(strcmp(console_args[0], "rm") == 0)
    {
       while(consoleBuf[bufP] == ' ')
            bufP++;
        
        argP = 0;
        while(consoleBuf[bufP] != 0)
        {
            console_args[1][argP++] = consoleBuf[bufP];
            bufP++;
        }
        console_args[1][argP] = 0;
       
        int k;
        for(k = 0; k < 20; k++)
        {
            if(((struct File*)curDir->directP[k]) != 0)
            {
                if(strcmp(console_args[1], ((struct File*)curDir->directP[k])->name) == 0)
                {
                    struct File *f1 = (struct File*)curDir->directP[k];
                    if(f1->type == 20)
                    {
                        delete_file(f1);
                        curDir->directP[k] = 0;  
                        myPrintf("  文件 %s 删除成功！\n\n", console_args[1]); 
                        write_to_block(2, &bitMapBlock);
                        write_to_block(1, f);                   
                    }
                    else if(f1->type == 10)
                    {
                        delete_dir(f1);
                        curDir->directP[k] = 0;
                        myPrintf("  目录 %s 删除成功！\n\n", console_args[1]);
                        write_to_block(2, &bitMapBlock);
                        write_to_block(1, f);
                    }
                    break;
                }
            }
        }
        if(k >= 10)
        {
            myPrintf("  %s: 文件不存在！\n\n", console_args[1]);
        }
         
    }
    else
    {
        myPrintf("%s: command not found!\n", console_args[0]);
    }
    return 0;
}


int strcmp(char *p, char *q)
{
	while (*p && *p == *q)
		p++, q++;
	if ((unsigned int)*p < (unsigned int)*q)
		return -1;
	if ((unsigned int)*p > (unsigned int)*q)
		return 1;
	return 0;
}


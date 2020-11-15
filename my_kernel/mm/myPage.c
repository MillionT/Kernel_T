#include "myPrintf.h"
#include "myPages.h"
#include "myTypes.h"
#include "myProcess.h"
#include "myFs.h"

unsigned long totalMEM;
unsigned long pagesNum;
unsigned long curMemoryPoint;
unsigned long PADDRMAX;


extern int mCONTEXT;
unsigned long end;

extern struct Page* listHead;
extern struct Page* listTail;

extern struct Page *myPages;
extern struct Process *myProcesses;
extern struct Process *curProcess;



void clear_to_zero(void *s, int len)
{
	void *M;
	M = s + len;
	while(s + 3 < M)
	{
		*(int *)s = 0;
		s += 4;
	}
	while(s < M)
	{
		*(char *)s++ = 0;
	}
}

void copy_from_src(void *src, void *dst, int len)
{
	void *M;
	M= dst + len;
	while (dst + 3 < M) {
		*(int *)dst = *(int *)src;
		dst += 4;
		src += 4;
	}
	while (dst < M) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}
}




void mmu_init()
{
    totalMEM = 64 * 1024 * 1024; //模拟64M内存
    pagesNum = totalMEM / (PAGESIZE);
    curMemoryPoint = 0; //当前可分配内存的虚拟地址
    PADDRMAX = totalMEM;//最大物理地址其实就是totalMem
    myPrintf("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
    myPrintf("the max physic address is %#08x\n", PADDRMAX);
    myPrintf("the total size of memory is %d M\n", totalMEM / (1024 * 1024));
    myPrintf("the max number of pages is %d K \n", pagesNum / 1024);
    myPrintf("now the free memory point is at %#08x\n", curMemoryPoint);
    myPrintf("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
    pages_manage_init();
    //page_function_test();
    page_init();
}


/*
    分页机制建立前的内存分配方式
*/
void *memory_alloc(unsigned int size)
{
    extern int _end;
    unsigned long pageBegin;
    end = 0x80400000;
    if(curMemoryPoint == 0)
    {
        curMemoryPoint = (unsigned long)(end);
        // myPrintf("curMemoryPoint = end, curMemoryPoint = %x\n", curMemoryPoint);
    }
    
    //先使对齐，向上取整
    curMemoryPoint = ((unsigned long)(curMemoryPoint) + PAGESIZE - 1 ) & (~(PAGESIZE - 1));
    
    pageBegin = curMemoryPoint;
    curMemoryPoint += size;
    // myPrintf("此时curMemoryPoint = %x, pageBegin = %x\n", curMemoryPoint, pageBegin);

    //清0
    clear_to_zero((void *)pageBegin, size);

    if(to_p_addr(curMemoryPoint) >= PADDRMAX)
    {
        kernel_panic(__FILE__, __LINE__);
        return (void *)-1; 
    }
   
    
//    myPrintf("memory_alloc中的to_p_addr成功\n");
    return (void *)pageBegin;//此时pageBegin
}

//分页机制建立前获取虚拟地址对应二级页表项地址
unsigned long *get_pgTableLevel2_item_init(unsigned long *pgTableLevel1_dir, unsigned long v_addr)
{
    
    unsigned long *pgTableLevel1_item;
    unsigned long *pgTableLevel2_dir;
    unsigned long *pgTableLevel2_item;
    
    //获得一级页表项
    pgTableLevel1_item = (unsigned long *)&pgTableLevel1_dir[((v_addr >> 22) & 0x03ff)];
    pgTableLevel2_dir = to_v_addr((unsigned long)(*pgTableLevel1_item) & ~0xfff);

    if(((*pgTableLevel1_item) & valid_bit) == 0x0)
    {
        pgTableLevel2_dir = (unsigned long*)memory_alloc(PAGESIZE);

        *pgTableLevel1_item = to_p_addr(pgTableLevel2_dir) | valid_bit;
        // myPrintf("getItem2init to_p_addr成功\n");
    }

    pgTableLevel2_item = (unsigned long *)&pgTableLevel2_dir[(v_addr >> 12) & 0x03ff];
    return pgTableLevel2_item;
}

//分页机制建立前的地址映射
void addr_space_setup(unsigned long *pgTableLevel1_dir, unsigned long p_addr, unsigned long v_addr, unsigned long size, int permission)
{

    unsigned long *pgTableLevel2_item;
    if(size % PAGESIZE != 0)
        kernel_panic( __FILE__, __LINE__);
    int j;
    for(j = 0; j < size / PAGESIZE; j++)
    {
        int tmp = j * PAGESIZE;
        pgTableLevel2_item = get_pgTableLevel2_item_init(pgTableLevel1_dir, v_addr + tmp);
        *pgTableLevel2_item = (p_addr + tmp) | permission | valid_bit;
    }
    
    //设置一级页表项的权限位
    pgTableLevel1_dir[(v_addr >> 22) & 0x03ff] |= permission | valid_bit;
}

unsigned long *for_test_pgTableLevel1_dir;

//建立二级页表机制
void pages_manage_init()
{
    myPrintf("\n\n\n****************************\n\n\n");
    myPrintf("\n开始建立二级页表映射机制! \n");
    myPrintf("共有 %d K个物理页面\n", pagesNum / 1024);//16K * 4K = 64M
    
    unsigned long *pgTableLevel1_dir;//一级页表
    pgTableLevel1_dir = (unsigned long *)memory_alloc(PAGESIZE);

    myPrintf("创建完成一级页表!\n");
    myPrintf("now the free memory point is at %#08x\n", curMemoryPoint);
    mCONTEXT = (int)pgTableLevel1_dir;

    for_test_pgTableLevel1_dir = pgTableLevel1_dir;
    setUpPgTableLevel1_dir = pgTableLevel1_dir;

    myPrintf("执行myPages部分的分配，sizeof(struct Page) = %d B\n", sizeof(struct Page));
    
    myPages = (struct Page*)memory_alloc(pagesNum * sizeof(struct Page));
    
    myPrintf("now the free memory point is at %#08x\n", curMemoryPoint);
    //对myPages结构占的内存进行向上取整
    int size_my_pages = (((unsigned long)(pagesNum * sizeof(struct Page)))+ PAGESIZE - 1) & (~(PAGESIZE - 1));
    
    myPrintf("size_my_pages = %d 个页面\n", size_my_pages / PAGESIZE);
    myPrintf("myPages的虚拟地址：%#08x\n", myPages);
    myPrintf("将虚拟地址U_MY_PAGES映射到物理地址myPages上\n");
    addr_space_setup(pgTableLevel1_dir, to_p_addr(myPages), U_MY_PAGES, size_my_pages, read_bit);
    
    myPrintf("curMemroyPoint is at: %#08x\n", curMemoryPoint);
    unsigned long* itm2 = get_pgTableLevel2_item_init(pgTableLevel1_dir, U_MY_PAGES);
    
    myPrintf("U_MY_PAGES对应的一级页表项: %#08x, 对应的二级页表项为: %#08x", pgTableLevel1_dir[(U_MY_PAGES >> 22) & 0x03ff], *itm2);
    myPrintf("\n\n\n****************************\n\n\n");

    myPrintf("执行myProcesses部分的分配， sizeof(struct Process) = %d B\n", sizeof(struct Process));
    myProcesses = (struct Process*)memory_alloc(processesNums * sizeof(struct Process));
    myPrintf("now the free memory point is at %#08x\n", curMemoryPoint);
    int size_my_processes = (((unsigned long)(processesNums * sizeof(struct Process)))+ PAGESIZE - 1) & (~(PAGESIZE - 1));
    myPrintf("size_my_processes = %d 个页面\n", size_my_processes / PAGESIZE);
    myPrintf("myProcesses的虚拟地址：%#08x\n", myProcesses);
    myPrintf("将虚拟地址U_MY_PROCESSES映射到物理地址myProcesses上\n");
    addr_space_setup(pgTableLevel1_dir, to_p_addr(myProcesses), U_MY_PROCESSES, size_my_processes, read_bit);
    
    myPrintf("curMemroyPoint is at: %#08x\n", curMemoryPoint);
    itm2 = get_pgTableLevel2_item_init(pgTableLevel1_dir, U_MY_PROCESSES);
    
    myPrintf("U_MY_PROCESSES对应的一级页表项: %#08x, 对应的二级页表项为: %#08x", pgTableLevel1_dir[(U_MY_PROCESSES >> 22) & 0x03ff], *itm2);
    myPrintf("\n\n\n****************************\n\n\n");

    myPrintf("执行myFiles部分的分配， sizeof(struct File) = %d B\n", sizeof(struct File));
    f = (struct File*)memory_alloc(filesNum * sizeof(struct File));
    myPrintf("now the free memory point is at %#08x\n", curMemoryPoint);
    int size_my_files = (((unsigned long)(filesNum * sizeof(struct File)))+ PAGESIZE - 1) & (~(PAGESIZE - 1));
    myPrintf("size_my_processes = %d 个页面\n", size_my_files / PAGESIZE);
    myPrintf("myFiles的虚拟地址：%#08x\n", f);
    
    myPrintf("\n\n\n****************************\n\n\n");
}



void page_init()
{
    myPrintf("page_init()函数开始执行\n");
    //空闲链表的初始化
    curMemoryPoint = ((unsigned long)(curMemoryPoint) + PAGESIZE - 1 ) & (~(PAGESIZE - 1));
    //标记到目前为止所有已经用过的pages，包括页目录和myPages和myProcesses，以及myFiles
    int i;
    pages_freelist_init();
    // myPrintf("pages_freelist_init()\n");

    int used_pages_num = to_p_addr(curMemoryPoint) / PAGESIZE;
    myPrintf("used_pages_num = %d\n", used_pages_num);
    for(i = 0; i < used_pages_num; i++)
        myPages[i].count = 1;
    for(i = used_pages_num; i < pagesNum; i++)
    {
        myPages[i].count = 0;
        list_insert(&myPages[i]); 
    }
    myPrintf("完成空闲物理内存链表的初始化!\n");
    // myPrintf("&myPages[pagesNum-1] = %#08x\n", &myPages[pagesNum - 1]);
    // myPrintf("listTail = %#08x\n", listTail);
    myPrintf("page_init()函数执行成功\n");
    myPrintf("\n\n\n****************************\n\n\n");
}


void page_init_after_test()
{
    
    curMemoryPoint = ((unsigned long)(curMemoryPoint) + PAGESIZE - 1 ) & (~(PAGESIZE - 1));
    int i;
    pages_freelist_init();
    // myPrintf("pages_freelist_init()\n");
    int used_pages_num = to_p_addr(curMemoryPoint) / PAGESIZE;
    // myPrintf("used_pages_num = %d\n", used_pages_num);
    for(i = 0; i < used_pages_num; i++)
        myPages[i].count = 1;
    for(i = used_pages_num; i < pagesNum; i++)
    {
        myPages[i].count = 0;
        list_insert(&myPages[i]); 
    }
    
}



//获得二级页表项指针
int getTableLevel2_item_running(unsigned long *pgTableLevel1_dir, unsigned long v_addr, unsigned long **ret, int flag)
{
    unsigned long *pgTableLevel1_item, *pgTableLevel2_dir, *pgTableLevel2_item;
    struct Page *p;

    pgTableLevel1_item = (unsigned long *)&(pgTableLevel1_dir[(v_addr >> 22) & 0x03ff]);

    pgTableLevel2_dir = to_v_addr((*pgTableLevel1_item) & ~0x0fff);

    //当v没有对应的二级页表时，先创建一个二级页表
    if((*pgTableLevel1_item & valid_bit) == 0x0)
    {
        if(flag)
        {
            //分配物理页
            int tmp = free_page_alloc(&p); 
            if(tmp == 0)
            {
                unsigned long p_addr_p = (p - myPages) << 12;
                unsigned long v_addr_p = to_v_addr(p_addr_p);
                //二级页表是虚拟地址
                pgTableLevel2_dir = v_addr_p;
                
                *pgTableLevel1_item = p_addr_p | valid_bit;
                // myPrintf("getItemrunning to_p_addr成功\n");
                p->count++;
            }
            else
            {
                *ret = 0x0;//此时分配失败
                // kernel_panic(__FILE__, __LINE__);
                return -3;
            }
            
        }
        else
        {
            *ret = 0x0;
            return 0; 
        }    
    }
    
    pgTableLevel2_item = (unsigned long *)&pgTableLevel2_dir[(((unsigned long)(v_addr)) >> 12) & 0x03ff];
    
    *ret = pgTableLevel2_item;
    return 0;
}


struct Page* find_page(unsigned long pgTableLevel1_dir, unsigned long v_addr, unsigned long **ret)
{
    struct Page *p;
    unsigned long *pgTableLevel2_item;

    getTableLevel2_item_running(pgTableLevel1_dir, v_addr, &pgTableLevel2_item, 0);

    //如果页表项不存在或者无效
    if(pgTableLevel2_item == 0)
        return 0;
    if(((*pgTableLevel2_item) & valid_bit) == 0)
        return 0;
    
    int itemPageId = (*pgTableLevel2_item) >> 12;
    if(itemPageId > pagesNum)
    {
        kernel_panic( __FILE__, __LINE__);
    }
    else
    {
        p = &myPages[itemPageId];
    }
    
    if(ret)
    {
        *ret = pgTableLevel2_item;
    }
    return p;
}

//取消va在二级页表中的映射地址，相应的page是否加入到空闲链表进行判断
void unmap(unsigned long *pgTableLevel1_dir, unsigned long v_addr)
{
    unsigned long *pgTableLevel2_item;
    struct Page* p;

    p = find_page(pgTableLevel1_dir, v_addr, &pgTableLevel2_item);
    //p为0时表示二级页表项不存在，不需要取消映射
    if(p == 0)
    {
        return;
    }
    p->count--;
    if(p->count == 0)
    {
        page_add_to_free_list(p);
    }
    
    *pgTableLevel2_item = 0;
    

    return;
}


int page_map_running(unsigned long *pgTableLevel1_dir, int permission, unsigned long v_addr, struct Page* p)
{
    unsigned int PERMISSION;
    unsigned long *pgTableLevel2_item;
    PERMISSION = permission | valid_bit;


    getTableLevel2_item_running(pgTableLevel1_dir, v_addr, &pgTableLevel2_item, 0);

   
    if((pgTableLevel2_item != 0x0) && (((*pgTableLevel2_item) & valid_bit ) != 0))
    {
       
        
        int itemPageId = (*pgTableLevel2_item) >> 12;

        if(itemPageId >= pagesNum)
        {
            kernel_panic(__FILE__, __LINE__);
        }
        else
        {
            struct Page *ptmp = &myPages[itemPageId];
            if(ptmp != p)
            {
                unmap(pgTableLevel1_dir, v_addr);
            }
            else
            {
                
                *pgTableLevel2_item = ((p - myPages) << 12 | PERMISSION);
                return 0;
            }    
        }
        
    }

    
    if(getTableLevel2_item_running(pgTableLevel1_dir, v_addr, &pgTableLevel2_item, 1) != 0)
    {
        return -4;
    }


    *pgTableLevel2_item = ( (p - myPages) << 12 | PERMISSION);
    p->count++;


    return 0;
}


void page_function_test()
{
    struct Page *p1, *p2, *p3, *p4, *p5, *p6;
    p1 = &myPages[pagesNum - 1];
    unsigned long* pgTableLevel1_dir;
    unsigned long* pgTableLevel2_dir;
    unsigned long* pgTableLevel2_item;
    struct Page *ptmp;
    myPrintf("\n\n\n****************************\n\n\n");
    myPrintf("开始进行分页管理功能的测试!\n");
    pages_freelist_init();
    if(isEmtpy())
        myPrintf("the free page list is now empty\n");
    if((free_page_alloc(&ptmp) < 0))
        myPrintf("当前链表为空，无法进行空闲页的分配!\n");
    
    p1 = &myPages[pagesNum - 1];
    p2 = &myPages[pagesNum - 2];
    p3 = &myPages[pagesNum - 3];
    p4 = &myPages[pagesNum - 4];
    p5 = &myPages[pagesNum - 5];
    p6 = &myPages[pagesNum - 6];
    p1->count = 0;
    p2->count = 0;
    p3->count = 0;
    p4->count = 0;
    p5->count = 0;
    page_add_to_free_list(p1);
    myPrintf("执行page_add_to_free_list(p1)后\n");
    if(free_page_alloc(&ptmp) == 0)
        myPrintf("当前空闲链表可以进行空闲页的分配!\n");
    
    page_add_to_free_list(ptmp);
    if(ptmp == p1)
        myPrintf("ptmp == p1\n\n");

    if(page_map_running(for_test_pgTableLevel1_dir, 0, 3 * PAGESIZE, p2) == 0)
        myPrintf("将虚拟地址 %#08x 与物理页p2产生映射，并将相应关系写到页表机制中\n", 3 * PAGESIZE);
    myPrintf("此时 ptemp 也就是 p1 作为二级页表\n");
    myPrintf("p1对应页的物理地址为: %#08x\n", (p1 - myPages) * PAGESIZE);
    myPrintf("虚拟地址 %#08x 对应的一级页表项表示的二级页表的物理地址为: %#08x\n", 3 * PAGESIZE, v_addr2pgTableLevel1_item(for_test_pgTableLevel1_dir, 3 * PAGESIZE));
    if(v_addr2pgTableLevel1_item(for_test_pgTableLevel1_dir, 3 * PAGESIZE) == (unsigned long)((p1 - myPages) * PAGESIZE))
        myPrintf("经过判断，p1被用来作为相应的二级页表!\n");
    myPrintf("在页表机制中寻找虚拟地址 %#08x 对应的二级页表项代表的页物理地址为 %#08x\n", 3 * PAGESIZE, v_addr2p_addr(for_test_pgTableLevel1_dir, 3 * PAGESIZE));
    if(v_addr2p_addr(for_test_pgTableLevel1_dir, 3 * PAGESIZE) == (unsigned long)((p2 - myPages) * PAGESIZE))
        myPrintf("经过判断，该物理地址是p2对应页的物理地址\n\n");

    if(page_map_running(for_test_pgTableLevel1_dir, 0, 4 * PAGESIZE, p3) == 0)
        myPrintf("将虚拟地址 %#08x 与物理页p3产生映射，并将相应关系写到页表机制中\n", 4 * PAGESIZE);
    myPrintf("虚拟地址 %#08x 对应的一级页表项表示的二级页表的物理地址为: %#08x\n", 4 * PAGESIZE, v_addr2pgTableLevel1_item(for_test_pgTableLevel1_dir, 4 * PAGESIZE));
    if(v_addr2pgTableLevel1_item(for_test_pgTableLevel1_dir, 4 * PAGESIZE) == (unsigned long)((p1 - myPages) * PAGESIZE))
        myPrintf("经过判断，p1被用来作为相应的二级页表!\n");
    myPrintf("在页表机制中寻找虚拟地址 %#08x 对应的二级页表项代表的页物理地址为 %#08x\n", 4 * PAGESIZE, v_addr2p_addr(for_test_pgTableLevel1_dir, 4 * PAGESIZE));
    if(v_addr2p_addr(for_test_pgTableLevel1_dir, 4 * PAGESIZE) == (unsigned long)((p3 - myPages) * PAGESIZE))
        myPrintf("经过判断，该物理地址是p3对应页的物理地址\n\n");
    
    if(page_map_running(for_test_pgTableLevel1_dir, 0, 2 * BLOCKSIZE, p5) < 0)
        myPrintf("将虚拟地址 %#08x 与物理页p5产生映射失败\n", 2 * BLOCKSIZE);
    myPrintf("原因在于当前空闲链表为空，无法分配相应的物理页作为二级页表\n");
    page_add_to_free_list(p4);
    myPrintf("执行page_add_to_free_list(p4)后\n");
    myPrintf("当前空闲链表可以进行空闲页的分配!\n");
    myPrintf("此时 ptemp 也就是 p4 作为二级页表\n\n");
    if(page_map_running(for_test_pgTableLevel1_dir, 0, 2 * BLOCKSIZE, p5) == 0)
        myPrintf("将虚拟地址 %#08x 与物理页p5产生映射，并将相应关系写到页表机制中\n", 2 * BLOCKSIZE);
    myPrintf("虚拟地址 %#08x 对应的一级页表项表示的二级页表的物理地址为: %#08x\n", 2 * BLOCKSIZE, v_addr2pgTableLevel1_item(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE));
    if(v_addr2pgTableLevel1_item(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE) == (unsigned long)((p4 - myPages) * PAGESIZE))
        myPrintf("经过判断，p4被用来作为相应的二级页表!\n");
    myPrintf("在页表机制中寻找虚拟地址 %#08x 对应的二级页表项代表的页物理地址为 %#08x\n", 2 * BLOCKSIZE, v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE));
    if(v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE) == (unsigned long)((p5 - myPages) * PAGESIZE))
        myPrintf("经过判断，该物理地址是p5对应页的物理地址\n\n");
    
    //使得多个虚拟地址映射到p3上,p5会自动unmap
    if(page_map_running(for_test_pgTableLevel1_dir, 0, 2 * BLOCKSIZE, p3) == 0)
        myPrintf("将虚拟地址 %#08x 与物理页p3产生映射，并将相应关系写到页表机制中\n", 2 * BLOCKSIZE);
    myPrintf("在页表机制中寻找虚拟地址 %#08x 对应的二级页表项代表的页物理地址为 %#08x\n", 2 * BLOCKSIZE, v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE));
    if(v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE) == (unsigned long)((p3 - myPages) * PAGESIZE))
        myPrintf("经过判断，该物理地址是p3对应页的物理地址\n");
    myPrintf("p3->count = %d\n", p3->count);
    
    myPrintf("p5->count = %d\n\n", p5->count);

    //重复映射，会修改permission，但是对页的引用次数是不发生改变的
    if(page_map_running(for_test_pgTableLevel1_dir, 0, 2 * BLOCKSIZE, p3) == 0)
        myPrintf("将虚拟地址 %#08x 与物理页p3产生映射，并将相应关系写到页表机制中\n", 2 * BLOCKSIZE);
    myPrintf("在页表机制中寻找虚拟地址 %#08x 对应的二级页表项代表的页物理地址为 %#08x\n", 2 * BLOCKSIZE, v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE));
    if(v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE) == (unsigned long)((p3 - myPages) * PAGESIZE))
        myPrintf("经过判断，该物理地址是p3对应页的物理地址\n");
    myPrintf("p3->count = %d\n", p3->count);
   
    myPrintf("p5->count = %d\n\n", p5->count);

    //测试unmap()函数
    unmap(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE);
    myPrintf("取消 %#08x的映射关系后，此时找到的对应的二级页表项为 %#08x, 表示该页表项无效\n", 2 * BLOCKSIZE, v_addr2p_addr(for_test_pgTableLevel1_dir, 2 * BLOCKSIZE));

    pgTableLevel2_dir = to_v_addr((p4 -  myPages) << 12);
    unsigned long v_addr = 2 * BLOCKSIZE;
    myPrintf("p4相应页表项的内容为: %#08x, 表示该页表项被清0\n", pgTableLevel2_dir[(v_addr >> 12) && 0x03ff]);
    myPrintf("p3->count = %d\n\n", p3->count);
    

    //取消p2,p3的全部映射
    unmap(for_test_pgTableLevel1_dir, 3 * PAGESIZE);
    unmap(for_test_pgTableLevel1_dir, 4 * PAGESIZE);

    
    pgTableLevel1_dir[0] = 0;
    pgTableLevel1_dir[1] = 0;
    pgTableLevel1_dir[2] = 0;

    p1->count = 0;
    p4->count = 0;
    page_add_to_free_list(p1);
    page_add_to_free_list(p4);
    myPrintf("取消全部页的映射关系后\n");
    myPrintf("p1->count = %d, p2->count = %d, p3->count = %d, p4->count = %d, p5->count = %d\n", p1->count, p2->count, p3->count, p4->count, p5->count);


    // int k;
    // for(k = 0; k < 5; k++)
    // {
    //     free_page_alloc(&ptmp);
    // }
    //到此为止链表为空，所有物理页仍处于可以被引用的状态
    // if(isEmtpy())
    //     myPrintf("当前空闲页链表为空！\n\n");
    myPrintf("分页管理功能测试成功！\n");
    myPrintf("\n\n\n****************************\n\n\n");
}

//获取虚拟地址对应的一级页表项代表的二级页表物理地址，如果无效返回~0
unsigned long v_addr2pgTableLevel1_item(unsigned long *pgTableLevel1_dir, unsigned long v_addr)
{
    unsigned long *pgTableLevel1_item = &pgTableLevel1_dir[(v_addr >> 22) & 0x03ff];
    if(!(*pgTableLevel1_item & valid_bit))
        return ~0;
    //后12位清0
    return (unsigned long)((*pgTableLevel1_item) & ~0x0fff);
}




unsigned long v_addr2p_addr(unsigned long *pgTableLevel1_dir, unsigned long v_addr)
{
	unsigned long *pgTableLevel2_dir;
	unsigned long *pgTableLevel1_item = &pgTableLevel1_dir[((unsigned long)v_addr >> 22) & 0x03ff];
	if (!(*pgTableLevel1_item & valid_bit)) 
    {
		return ~0;
	}
	pgTableLevel2_dir = (unsigned long *)to_v_addr((unsigned long)(*pgTableLevel1_item) & ~0x0fff);
    unsigned long *pgTableLevel2_Item = &pgTableLevel2_dir[((unsigned long)(v_addr) >> 12) & 0x03ff];
	if (!(*pgTableLevel2_Item & valid_bit)) {
		return ~0;
	}
    
	return (*pgTableLevel2_Item & ~0x0fff);
}




void page_add_to_free_list(struct Page* p)
{
    //只要尚被占用，就不会加到free list中
    if(p->count > 0)
    {
        kernel_panic(__FILE__, __LINE__);
        return;
    }
    else if(p->count == 0)
    {
        list_insert(p);
        //插入到队首
        // p->nxtPage = listHead->nxtPage;
        // listHead->nxtPage->prePage = p;
        // p->prePage = listHead;
        // listHead->nxtPage = p;
        return;
    }
    kernel_panic(__FILE__, __LINE__);
}

int free_page_alloc(struct Page **p_p)
{
    
    struct Page *p;
    if(!isEmtpy())
    {
        *p_p = listTail;//*p_p是返回值
        p = listTail->prePage;
        listTail = p;
        listTail->nxtPage = NULL;

        struct Page *t;
        t = *p_p;
        t->count = 0; 
       
        unsigned long p_addr_p_p = (t - myPages) << 12;
        unsigned long v_addr_p_p = to_v_addr(p_addr_p_p);
        // myPrintf("free_page_alloc to_v_addr: %x\n", v_addr_p_p);
        //从虚拟地址开始的4KB清0
        clear_to_zero((void *)v_addr_p_p, 4096); 
        
        return 0;
    }
    else
    {
        return -1;
    }   
}

void list_insert(struct Page* p)
{
    listTail->nxtPage = p;
    p->prePage = listTail;
    listTail = p;
    p->nxtPage = NULL;
    p->count = 0; 
}


struct Page listHeadPage;
void pages_freelist_init()
{
    listHead = &listHeadPage;
    // myPrintf("&listHeadPage = %#08x\n", &listHeadPage);
    // myPrintf("listHead = %#08x\n", listHead);
    listHead->nxtPage = NULL;
    listHead->prePage = NULL;
    listHead->count = 0;
    listTail = listHead;
    // myPrintf("空闲链表初始化执行成功!\n");
}

int isEmtpy()
{
    if(listHead == listTail)
        return 1;
    else
    {
        return 0;
    }
}


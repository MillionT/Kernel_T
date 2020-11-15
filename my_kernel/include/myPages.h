#ifndef _MY_PAGES_H
#define _MY_PAGES_H

#include "myPrintf.h"
#include "myMemory.h"
#include "myTypes.h"

struct Page{
    //指向空闲链表上一个Page的指针
    struct Page* prePage;
    //指向空闲链表下一个Page的指针
    struct Page* nxtPage;
    //当前Page引用计数
    int count;
};

struct Page* listHead;
struct Page* listTail;

struct Page *myPages;

unsigned long *setUpPgTableLevel1_dir;

void pages_freelist_init();

int isEmtpy();

void list_insert(struct Page* p);

int free_page_alloc(struct Page **p_p);

void page_add_to_free_list(struct Page* p);

unsigned long v_addr2p_addr(unsigned long *pgTableLevel1_dir, unsigned long v_addr);

void *memory_alloc(unsigned int size);

unsigned long *get_pgTableLevel2_item_init(unsigned long *pgTableLevel1_dir, unsigned long v_addr);

void addr_space_setup(unsigned long *pgTableLevel1_dir, unsigned long p_addr, unsigned long v_addr, unsigned long size, int permission);

void pages_manage_init();

int getTableLevel2_item_running(unsigned long *pgTableLevel1_dir, unsigned long v_addr, unsigned long **ret, int flag);

struct Page* find_page(unsigned long pgTableLevel1_dir, unsigned long v_addr, unsigned long **ret);

void unmap(unsigned long *pgTableLevel1_dir, unsigned long v_addr);

int page_map_running(unsigned long *pgTableLevel1_dir, int permission, unsigned long v_addr, struct Page* p);

void mmu_init();

void page_function_test();

void page_init();

void page_init_after_test();

unsigned long v_addr2pgTableLevel1_item(unsigned long *pgTableLevel1_dir, unsigned long v_addr);

#endif
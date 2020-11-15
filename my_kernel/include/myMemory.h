#ifndef _MY_MEMORY_H
#define _MY_MEMORY_H


#define PAGESIZE 4096 //每页4KB
#define BLOCKSIZE (4 * 1024 * 1024) //4MB
#define K_START 0x80000000
#define U_MY_PAGES (0x80000000 - (2 * 4 * 1024 * 1024))
//64M为8400 0000
#define MY_TIMESTACK 0x82000000 

/*权限位*/
#define valid_bit 0x0200 //有效权限位
#define read_bit 0x0400 //只读权限位


extern unsigned long pagesNum;

unsigned long to_p_addr(unsigned long v_addr);

unsigned long to_v_addr(unsigned long p_addr);

void clear_to_zero(void *b, int len);

void copy_from_src(void *src, void *dst, int len);


#endif
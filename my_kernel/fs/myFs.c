#include <myFs.h>
#include <myPages.h>



char *strcpy(char *dst, const char *src)
{
	char *ret;
	ret = dst;
	while ((*dst++ = *src++) != 0);
	return ret;
}

void memset(void *src, int num, int size)
{
    int *tmp = (int *)src;
    while(size-- > 0)
    {
        *tmp = num;
        tmp++;
    }
}


void init_fs()
{
    super.magicNumber = 0x1111;
    super.nBlocks = 1024;
    
    super.root = &f[0];
    // read_from_block(1, f);
    // read_from_block(2, &bitMapBlock);
    curDir = super.root;
    bitmap = (int *)&bitMapBlock;
    // myPrintf("root->name=%s\n",super.root->name);
    
    // fs_begin();//第一次执行时通过这个函数初始化文件系统
    
    
    // read_from_block(2, &bitMapBlock);
    // read_from_block(1, f);
    // myPrintf("写入成功！\n");

    
    
    // myPrintf("&bitMapBlock = %#08x\n", &bitMapBlock);
    // myPrintf("%32b\n", bitmap[0]); //4B
    // myPrintf("%#8x\n", bitmap[1]); 
    // myPrintf("%#08x\n", bitmap[32]); //1024/32 = 32所以
    // myPrintf("%#8x\n", bitmap[31]); 
    // myPrintf("sizeof(bitBlock) = %dB\n", sizeof(struct bitBlock));
    // myPrintf("sizeof(File) = %dB\n", sizeof(struct File));
    // write_to_block(2, &bitMapBlock);
    // read_from_block(2, &bitMapBlock);
    // myPrintf("%#8x\n", bitmap[1]); 
    // bitMap_read();
    
    myPrintf("\n/----------------------------------/\n\n");
	myPrintf("文件系统初始化成功!\n");
    myPrintf("文件系统最多可管理 1024 个block!\n");
	myPrintf("\n/----------------------------------/\n\n");
    
}

void fs_begin()
{
    super.root->FC = 3;
    super.root->directCount = 0;
    (super.root)->type = 10;
    strcpy(super.root->name, "/");

    bitmapBlockCount = 1;
    int i;
    for(i = 0; i < bitmapBlockCount; i++)
    {
        memset(bitMapBlock.data, 0xffffffff, 1024/32);//
        // myPrintf("here!\n");
    }
    if(bitmapBlockCount * 4096 != 1024)
    {
        int k = 1024 % 4096 / 32;
        memset(bitMapBlock.data + k, 0x00000000, 4096 / 4 - k);//1024 * 4B
    }

    write_to_block(2, &bitMapBlock);
    write_to_block(1, f);
}


void bitMap_read()
{
    struct Page *p;
    free_page_alloc(&p);
    unsigned long p_addr = (p - myPages) << 12;
    read_from_block(2, p_addr);
    void *pp_addr = (p - myPages) << 12;
    int t = 0;
    while(t != 33)
    {
        t++;
        if(t == 1)
            myPrintf("%#08x", (*(int *)pp_addr));
        pp_addr += 4;
    }
    // myPrintf("t = %d\n", t);
    page_add_to_free_list(p);
}


//返回1表示空闲
int block_is_free(int blockNo)
{
	if (blockNo >= super.nBlocks) {
		return 0;
	}

    
	if (bitmap[blockNo / 32] & (1 << (blockNo % 32))) {
		return 1;
	}

	return 0;
}

//分配block
int alloc_block_num()
{
	int blockNo;
	for (blockNo = 3; blockNo < super.nBlocks; blockNo++) 
    {
        //如果是空闲的
		if (bitmap[blockNo / 32] & (1 << (blockNo % 32))) 
        {
			bitmap[blockNo / 32] &= ~(1 << (blockNo % 32));
			return blockNo;
		}
	}
	return -3;
}

void block_add_to_free(int no)
{
    //如果不是空闲的
    // myPrintf("bitmap[0] = %32b\n",bitmap[0]);
    bitmap[no / 32] |= (1 << (no % 32));
    // myPrintf("bitmap[0] = %32b\n",bitmap[0]);
}




void read_from_block(int blockNo, unsigned long p_addr)
{
    if(blockNo >= super.nBlocks)
        kernel_panic(__FILE__, __LINE__);

    if(block_is_free(blockNo) && (blockNo != 2) && (blockNo != 1))
        kernel_panic(__FILE__, __LINE__);
    
    read_from_ide(0, blockNo * 4096 / 512, p_addr, 4096 / 512);
}

void write_to_block(int blockNo, unsigned long p_addr)
{
    write_to_ide(0, blockNo * 4096 / 512, p_addr, 4096 / 512);
}


void delete_file(struct File *f)
{
    int num = f->directP[0];
    f->directP[0] = 0;
    block_add_to_free(num);
    f->type = 0;
    f->size = 0;
    f->fDir = 0;
    f->directCount = 0;  
    // curDir->directP[k] = 0;                      
    
}

void delete_dir(struct File *f)
{
    int i;
    for(i = 0; i < f->directCount; i++)
    {
        if((f->directP[i]) != 0)
        {
            struct File *fTmp = (struct File*)f->directP[i];
            if(fTmp->type == 10)
            {
                delete_dir(fTmp);
                f->directP[i] = 0;
            }
            else if(fTmp->type == 20)
            {
                delete_file(fTmp);
                f->directP[i] = 0;
            }
        }
    }
    f->fDir = 0;
    f->directCount = 0;
    f->type = 0;
    return;
}

void create_file()
{
    f[2].fDir = curDir;
    // strcpy(f[2].name, "f1.txt");
    struct Page *p;
    free_page_alloc(&p);
    int cBlockNo = alloc_block_num();
    myPrintf("分配的BlockNo = %d\n", cBlockNo);
    f[2].directP[0] = cBlockNo;
    f[2].type = 20;//表示为文件
    f[2].directCount = 1;//表示被占用
    f[2].size = 0;//以B为单位
    int h;
    for(h = 1; h < 10; h++)
    {
        f[2].directP[h] = 0;
    }
    int i = curDir->directCount;
    curDir->directP[i] = (unsigned long)(&(f[2]));
    curDir->directCount++;
    if(i >= 20)
        kernel_panic(__FILE__, __LINE__);
    f[2].size = 7;
    void *p_addr = (p - myPages) << 12;
    char *inBuf = "这是第一个文件!";
    int inBufCount = 0;
    // myPrintf("%#08x\n", (unsigned long)p_addr);
    myPrintf("%c\n", *inBuf);
    while(*inBuf != 0)
    {
        myPrintf("h\n");
        *(char *)p_addr = *inBuf;
        inBuf++;
        p_addr += 1;
    }
    // for(i = 0; i < 7; i++)
    // {
    //     *(char *)p_addr = inBuf[i];
    //     p_addr += 1;
    // }    
    myPrintf("1\n");
    unsigned long p_add = (p - myPages) << 12;
    write_to_block(cBlockNo, p_add);
    page_add_to_free_list(p);
}

void create_dir()
{
    f[1].directCount = 0;
    f[1].type = 10;
    // strcpy(f[1].name, "dir1");
    f[1].fDir = curDir;
    int i = curDir->directCount;
    curDir->directP[i] = (unsigned long)(&(f[1]));
    curDir->directCount++;
    if(i >= 20)
        kernel_panic(__FILE__, __LINE__);
}








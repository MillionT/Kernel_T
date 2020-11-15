#include<myMemory.h>
#include<myPrintf.h>
#include<myFs.h>



extern int read_sector(int diskNum, int offset);
extern int write_sector(int diskNum, int offset);


void read_from_ide(int diskNum, int sectorNum, void *dst, int NSectors)
{
    int start = sectorNum * 0x200;
    int end = start + NSectors * 0x200;
    int offset = 0;
    while(start + offset < end)
    {
        //返回0失败，否则成功
        if(read_sector(diskNum, start + offset))
        {
            copy_from_src((void *)0x93004000, dst + offset, 0x200);
            offset += 0x200;
            // if(offset == 0x200 || offset == (4095 - 0x200))
                // myPrintf("here1!\n");
        }
        else
        {
            kernel_panic(__FILE__, __LINE__);
        }
        
    }
}

void write_to_ide(int diskNum, int sectorNum, void *src, int NSectors)
{
    int start = sectorNum * 0x200;
    int end = start + NSectors * 0x200;
    int offset = 0;
    while(start + offset < end)
    {
        copy_from_src(src + offset, (void*)0x93004000, 0x200);
        
        if(write_sector(diskNum, start + offset))
        {
            offset += 0x200;
        }
        else
        {
            kernel_panic(__FILE__, __LINE__);
        }
        
    }

}




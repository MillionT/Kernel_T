#ifndef _MY_FS_H
#define _MY_FS_H



struct File{
    char name[30];
    int size;
    int type;//10表示dir,20表示文件
    unsigned long directP[20];
    struct File *fDir;  //父目录
    int directCount;//当前文件有效
    int FC;//记录空闲文件控制块位置
};

struct File *f;
#define filesNum 50



struct SuperBlock{
    unsigned int magicNumber;//指明文件格式
    unsigned int nBlocks;
    struct File *root;//根目录   
};


struct bitBlock{
    int data[1024];//共4KB
};
struct bitBlock bitMapBlock;

int bitmapBlockCount;
int *bitmap;

void read_from_ide(int diskNum, int sectorNum, void *dst, int NSectors);
void write_to_ide(int diskNum, int sectorNum, void *src, int NSectors);
void write_to_block(int blockNo, unsigned long p_addr);
void read_from_block(int blockNo, unsigned long p_addr);

struct SuperBlock super;
struct File *curDir;

int alloc_block_num();
int block_is_free(int blockNo);
void init_fs();
char *strcpy(char *dst, const char *src);
void block_add_to_free(int no);
void delete_file(struct File *f);
void delete_dir(struct File *f);

void create_file();
void create_dir();
void bitMap_read();
void fs_begin();

#endif





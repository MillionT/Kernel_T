#ifndef _PRINTF_H_
#define _PRINTF_H_

#include <stdarg.h>

//定义最大输出长度
#define OUT_MAX_LEN 200

void myPrintf(char *fmt, ...);

void kernel_panic(char *curFile, int curLine);

void outNumFunctionP(unsigned long num, int outputWidth, 
int right, char alignCH, int isUp, int b, int flag);

void printFunction(char *str, int length);

void myPrintfTest();

#endif

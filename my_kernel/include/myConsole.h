#ifndef _Console_H_
#define _Console_H_


char consoleBuf[100];
int consoleCount;

int strcmp(char *p, char *q);

void console_init();

int console_read();


struct Process *runProcess[100];
int fCount;


#endif
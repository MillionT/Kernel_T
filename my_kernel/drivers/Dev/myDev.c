#include "dev.h"

#define	BASEADDR		((int)0x80000000)
#define	PRINT_ADDRESS	(BASEADDR + DEV_OUT_ADDRESS + DEV_OFFSET)

void kernel_printCH(char c)
{
	*((volatile unsigned char *) PRINT_ADDRESS) = c;
}





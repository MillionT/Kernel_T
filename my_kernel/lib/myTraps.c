#include <myTrap.h>
#include <myProcess.h>
#include <myMemory.h>

extern void handle_int();
extern void handle_reserved();
unsigned long my_exception_handlers[32];
void trap_init(){
	int i;
	for(i=0;i<32;i++)
		set_except_vector(i, handle_reserved);
	set_except_vector(0, handle_int);
}


void set_except_vector(int num, void *address)
{
	unsigned long newHandler = (unsigned long)address;
	my_exception_handlers[num] = newHandler;
}




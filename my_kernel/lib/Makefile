INCLUDES := -I./ -I../ -I../include/
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all:  myPrintf.o  myProcess.o mySched.o myClock.o myTraps.o excGenex.o myClock_asm.o getChar.o myConsole.o mySched_asm.o

clean:
	rm -rf *~ *.o


include ../include.mk

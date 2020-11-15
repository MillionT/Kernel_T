CROSS_COMPILE := /opt/eldk/usr/bin/mips_4KC-
CC			  := $(CROSS_COMPILE)gcc
CFLAGS		  := -O -G 0 -mno-abicalls -fno-builtin -Wa,-xgot -Wall -fPIC
LD			  := $(CROSS_COMPILE)ld





drivers_dir	  := drivers
boot_dir	  := boot
init_dir	  := init
lib_dir		  := lib
tools_dir	  := tools
mm_dir		  := mm
fs_dir		  := fs
vmlinux_elf	  := gxemul/vmkernel
link_script   := $(tools_dir)/myLds.lds
modules		  := boot drivers init lib mm fs
objects		  := $(boot_dir)/start.o			  \
				 $(init_dir)/main.o			  \
			   	 $(drivers_dir)/Dev/myDev.o  \
				 $(lib_dir)/*.o \
				 $(mm_dir)/*.o \
				 $(fs_dir)/*.o





.PHONY: all $(modules) clean

all: $(modules) vmlinux

vmlinux: $(modules)
	$(LD) -o $(vmlinux_elf) -N -T $(link_script) $(objects)

$(modules): 
	$(MAKE) --directory=$@

clean: 
	for d in $(modules);	\
		do					\
			$(MAKE) --directory=$$d clean; \
		done; \
	rm -rf *.o *~ $(vmlinux_elf)

include include.mk

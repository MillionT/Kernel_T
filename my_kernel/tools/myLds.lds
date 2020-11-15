OUTPUT_ARCH(mips)
ENTRY(my_kernel_entry)
SECTIONS
{
	. = 0x80000180;
  	.exceptVec : {*(.text.excVec)}
	. = 0x80010000;
	.text : {*(.text)}
	.data : {*(.data)}
	.bss : {*(.bss)}
	.sdata : {*(.sdata)}
	. = 0x80400000;
	_end = .;
}



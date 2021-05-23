
.set FLAGS,    0

.set MAGIC,    0x1BADB002

.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot

.long MAGIC
.long FLAGS
.long CHECKSUM

stackBottom:

.skip 4096

stackTop:

.section .text
.global _start
.type _start, @function


_start: #start

	mov $stackTop, %esp

	call kernel_entry

	cli


hltLoop:

	hlt
	jmp hltLoop

.size _start, . - _start






# RUN: llvm-mc -triple=sh3el-unknown-none-elf -filetype=obj %s -o %t
# RUN: llvm-readobj -r %t | FileCheck --check-prefix=RELOC %s
# RUN: llvm-objdump -s -j .data %t | FileCheck --check-prefix=DATA %s

# Like GNU as, the addend of a data relocation is in r_addend and in the
# section contents too: GNU ld reads it from the contents.

# RELOC:      R_SH_DIR32 x 0x8
# RELOC-NEXT: R_SH_DIR32 .rodata 0x4
# RELOC-NEXT: R_SH_REL32 x 0xC
# RELOC-NEXT: R_SH_DIR32 x 0x0

# DATA:      0000 08000000 04000000 0c000000 00000000

	.data
	.long	x+8
	.long	local+3
	.long	x+12-.
	.long	x

	.section .rodata
	.byte	0
local:
	.byte	1

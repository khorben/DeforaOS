.main
	/* 0x00 */
	push	%es
	push	%cs
	emms
	push	%fs
	cpuid
	push	%gs
	/* 0x10 */
	push	%ss
	push	%ds
	/* 0x20 */
	daa
	das
	/* 0x30 */
	aaa
	/* 0x50 */
	push	%eax
	push	%ecx
	push	%edx
	push	%ebx
	push	%esp
	push	%ebp
	/* 0x60 */
	pusha
	/* 0x90 */
	wait
	fwait
	/* 0xa0 */
	stosd
	scasd
	cmpsb
	/* 0xc0 */
	ret
	/* 0xd8 */
	fcom
	fcomp
	/* 0xd9 */
	fnop
	fchs
	fabs
	fldl2e
	fldpi
	fldlg2
	fldln2
	fldz
	f2xm1
	fptan
	fprem1
	fdecstp
	fprem
	fsqrt
	fsincos
	frndint
	fscale
	fsin
	fcos
	/* 0xda */
	fucompp
	/* 0xdb */
	fnclex
	/* 0xdd */
	fucom
	fucomp
	/* 0xde */
	faddp
	fcompp
	fsubrp
	ftst
	fsubp
	fdivp
	/* 0xf0 */
	cmc
	clc
	stc
	cli
	sti
	cld
	std

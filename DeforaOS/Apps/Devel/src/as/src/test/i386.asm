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
	fcom	%st0, %st0
	fcom	%st0, %st1
	fcom	%st0, %st2
	fcom	%st0, %st3
	fcom	%st0, %st4
	fcom	%st0, %st5
	fcom	%st0, %st6
	fcom	%st0, %st7
	fcomp	%st0, %st0
	fcomp	%st0, %st1
	fcomp	%st0, %st2
	fcomp	%st0, %st3
	fcomp	%st0, %st4
	fcomp	%st0, %st5
	fcomp	%st0, %st6
	fcomp	%st0, %st7
	/* 0xd8f0 */
	fdiv	%st0, %st0, %st0
	fdiv	%st0, %st1, %st0
	fdiv	%st0, %st2, %st0
	fdiv	%st0, %st3, %st0
	fdiv	%st0, %st4, %st0
	fdiv	%st0, %st5, %st0
	fdiv	%st0, %st6, %st0
	fdiv	%st0, %st7, %st0
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
	/* 0xdc */
	fdiv	%st0, %st0, %st0
	fdiv	%st1, %st0, %st1
	fdiv	%st2, %st0, %st2
	fdiv	%st3, %st0, %st3
	fdiv	%st4, %st0, %st4
	fdiv	%st5, %st0, %st5
	fdiv	%st6, %st0, %st6
	fdiv	%st7, %st0, %st7
	/* 0xdd */
	fucom	%st0
	fucom	%st1
	fucom	%st2
	fucom	%st3
	fucom	%st4
	fucom	%st5
	fucom	%st6
	fucom	%st7
	fucomp
	/* 0xde */
	faddp
	fcompp	%st0, %st1
	fsubrp
	ftst
	fsubp
	fdivp	%st0, %st0, %st0
	fdivp	%st1, %st0, %st1
	fdivp	%st2, %st0, %st2
	fdivp	%st3, %st0, %st3
	fdivp	%st4, %st0, %st4
	fdivp	%st5, %st0, %st5
	fdivp	%st6, %st0, %st6
	fdivp	%st7, %st0, %st7
	/* 0xf0 */
	cmc
	clc
	stc
	cli
	sti
	cld
	std

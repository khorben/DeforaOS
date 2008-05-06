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
	/* 0xd8c0 */
	fadd	%st0, %st0
	fadd	%st1, %st0
	fadd	%st2, %st0
	fadd	%st3, %st0
	fadd	%st4, %st0
	fadd	%st5, %st0
	fadd	%st6, %st0
	fadd	%st7, %st0
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
	ftst
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
	/* 0xdac0 */
	fcmovb	%st0, %st0
	fcmovb	%st0, %st1
	fcmovb	%st0, %st2
	fcmovb	%st0, %st3
	fcmovb	%st0, %st4
	fcmovb	%st0, %st5
	fcmovb	%st0, %st6
	fcmovb	%st0, %st7
	fcmove	%st0, %st0
	fcmove	%st0, %st1
	fcmove	%st0, %st2
	fcmove	%st0, %st3
	fcmove	%st0, %st4
	fcmove	%st0, %st5
	fcmove	%st0, %st6
	fcmove	%st0, %st7
	/* 0xdad0 */
	fcmovbe	%st0, %st0
	fcmovbe	%st0, %st1
	fcmovbe	%st0, %st2
	fcmovbe	%st0, %st3
	fcmovbe	%st0, %st4
	fcmovbe	%st0, %st5
	fcmovbe	%st0, %st6
	fcmovbe	%st0, %st7
	fcmovu	%st0, %st0
	fcmovu	%st0, %st1
	fcmovu	%st0, %st2
	fcmovu	%st0, %st3
	fcmovu	%st0, %st4
	fcmovu	%st0, %st5
	fcmovu	%st0, %st6
	fcmovu	%st0, %st7
	/* 0xdae0 */
	fucompp
	/* 0xdb */
	fnclex
	/* 0xdbc0 */
	fcmovnb	%st0, %st0
	fcmovnb	%st0, %st1
	fcmovnb	%st0, %st2
	fcmovnb	%st0, %st3
	fcmovnb	%st0, %st4
	fcmovnb	%st0, %st5
	fcmovnb	%st0, %st6
	fcmovnb	%st0, %st7
	fcmovne	%st0, %st0
	fcmovne	%st0, %st1
	fcmovne	%st0, %st2
	fcmovne	%st0, %st3
	fcmovne	%st0, %st4
	fcmovne	%st0, %st5
	fcmovne	%st0, %st6
	fcmovne	%st0, %st7
	/* 0xdbd0 */
	fcmovnbe	%st0, %st0
	fcmovnbe	%st0, %st1
	fcmovnbe	%st0, %st2
	fcmovnbe	%st0, %st3
	fcmovnbe	%st0, %st4
	fcmovnbe	%st0, %st5
	fcmovnbe	%st0, %st6
	fcmovnbe	%st0, %st7
	fcmovnu	%st0, %st0
	fcmovnu	%st0, %st1
	fcmovnu	%st0, %st2
	fcmovnu	%st0, %st3
	fcmovnu	%st0, %st4
	fcmovnu	%st0, %st5
	fcmovnu	%st0, %st6
	fcmovnu	%st0, %st7
	/* 0xdbe8 */
	fucomi	%st0, %st0
	fucomi	%st0, %st1
	fucomi	%st0, %st2
	fucomi	%st0, %st3
	fucomi	%st0, %st4
	fucomi	%st0, %st5
	fucomi	%st0, %st6
	fucomi	%st0, %st7
	/* 0xdbf0 */
	fcomi	%st0, %st0
	fcomi	%st0, %st1
	fcomi	%st0, %st2
	fcomi	%st0, %st3
	fcomi	%st0, %st4
	fcomi	%st0, %st5
	fcomi	%st0, %st6
	fcomi	%st0, %st7
	/* 0xdc */
	fdiv	%st0, %st0, %st0
	fdiv	%st1, %st0, %st1
	fdiv	%st2, %st0, %st2
	fdiv	%st3, %st0, %st3
	fdiv	%st4, %st0, %st4
	fdiv	%st5, %st0, %st5
	fdiv	%st6, %st0, %st6
	fdiv	%st7, %st0, %st7
	/* 0xdcc0 */
	fadd	%st0, %st0
	fadd	%st0, %st1
	fadd	%st0, %st2
	fadd	%st0, %st3
	fadd	%st0, %st4
	fadd	%st0, %st5
	fadd	%st0, %st6
	fadd	%st0, %st7
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
	fcompp	%st0, %st1
	fsubrp
	/* 0xdec0 */
	faddp	%st0, %st0
	faddp	%st0, %st1
	faddp	%st0, %st2
	faddp	%st0, %st3
	faddp	%st0, %st4
	faddp	%st0, %st5
	faddp	%st0, %st6
	faddp	%st0, %st7
	/* 0xdee0 */
	fsubp
	/* 0xdef0 */
	fdivp	%st0, %st0, %st0
	fdivp	%st1, %st0, %st1
	fdivp	%st2, %st0, %st2
	fdivp	%st3, %st0, %st3
	fdivp	%st4, %st0, %st4
	fdivp	%st5, %st0, %st5
	fdivp	%st6, %st0, %st6
	fdivp	%st7, %st0, %st7
	/* 0xdfe8 */
	fucomip	%st0, %st0
	fucomip	%st0, %st1
	fucomip	%st0, %st2
	fucomip	%st0, %st3
	fucomip	%st0, %st4
	fucomip	%st0, %st5
	fucomip	%st0, %st6
	fucomip	%st0, %st7
	/* 0xdff0 */
	fcomip	%st0, %st0
	fcomip	%st0, %st1
	fcomip	%st0, %st2
	fcomip	%st0, %st3
	fcomip	%st0, %st4
	fcomip	%st0, %st5
	fcomip	%st0, %st6
	fcomip	%st0, %st7
	/* 0xf0 */
	cmc
	clc
	stc
	cli
	sti
	cld
	std

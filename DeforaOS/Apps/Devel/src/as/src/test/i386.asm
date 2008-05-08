.main
	/* 0x00 */
	add	$0x90, %al
	add	$0x90, %eax
	push	%es
	push	%cs
	emms
	push	%fs
	cpuid
	push	%gs
	/* 0x0f */
	/* 0x0fba */
	bt	$0x90, %eax
	bt	$0x90, %ecx
	bt	$0x90, %edx
	bt	$0x90, %ebx
	bt	$0x90, %esp
	bt	$0x90, %ebp
	bt	$0x90, %esi
	bt	$0x90, %edi
	/* 0x0ff0 */
	btc	$0x90, %eax
	btc	$0x90, %ecx
	btc	$0x90, %edx
	btc	$0x90, %ebx
	btc	$0x90, %esp
	btc	$0x90, %ebp
	btc	$0x90, %esi
	btc	$0x90, %edi
	/* 0x10 */
	push	%ss
	push	%ds
	/* 0x10c0 */
	adc	%al, %al
	adc	%al, %cl
	adc	%al, %dl
	adc	%al, %bl
	adc	%al, %ah
	adc	%al, %ch
	adc	%al, %dh
	adc	%al, %bh
	adc	%cl, %al
	adc	%cl, %cl
	adc	%cl, %dl
	adc	%cl, %bl
	adc	%cl, %ah
	adc	%cl, %ch
	adc	%cl, %dh
	adc	%cl, %bh
	/* 0x10d0 */
	adc	%dl, %al
	adc	%dl, %cl
	adc	%dl, %dl
	adc	%dl, %bl
	adc	%dl, %ah
	adc	%dl, %ch
	adc	%dl, %dh
	adc	%dl, %bh
	adc	%bl, %al
	adc	%bl, %cl
	adc	%bl, %dl
	adc	%bl, %bl
	adc	%bl, %ah
	adc	%bl, %ch
	adc	%bl, %dh
	adc	%bl, %bh
	/* 0x11 */
	/* 0x11c0 */
	adc	%eax, %eax
	adc	%eax, %ecx
	adc	%eax, %edx
	adc	%eax, %ebx
	adc	%eax, %esp
	adc	%eax, %ebp
	adc	%eax, %esi
	adc	%eax, %edi
	adc	%ecx, %eax
	adc	%ecx, %ecx
	adc	%ecx, %edx
	adc	%ecx, %ebx
	adc	%ecx, %esp
	adc	%ecx, %ebp
	adc	%ecx, %esi
	adc	%ecx, %edi
	/* 0x11d0 */
	adc	%edx, %eax
	adc	%edx, %ecx
	adc	%edx, %edx
	adc	%edx, %ebx
	adc	%edx, %esp
	adc	%edx, %ebp
	adc	%edx, %esi
	adc	%edx, %edi
	adc	%ebx, %eax
	adc	%ebx, %ecx
	adc	%ebx, %edx
	adc	%ebx, %ebx
	adc	%ebx, %esp
	adc	%ebx, %ebp
	adc	%ebx, %esi
	adc	%ebx, %edi
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
	/* 0x80 */
	add	$0x90, %al
	add	$0x90, %cl
	add	$0x90, %dl
	add	$0x90, %bl
	add	$0x90, %ah
	add	$0x90, %ch
	add	$0x90, %dh
	add	$0x90, %bh
	adc	$0x90, %al
	adc	$0x90, %cl
	adc	$0x90, %dl
	adc	$0x90, %bl
	adc	$0x90, %ah
	adc	$0x90, %ch
	adc	$0x90, %dh
	adc	$0x90, %bh
	/* 0x81 */
	add	$0x90, %eax
	add	$0x90, %ecx
	add	$0x90, %edx
	add	$0x90, %ebx
	add	$0x90, %esp
	add	$0x90, %ebp
	add	$0x90, %esi
	add	$0x90, %edi
	adc	$0x90, %eax
	adc	$0x90, %ecx
	adc	$0x90, %edx
	adc	$0x90, %ebx
	adc	$0x90, %esp
	adc	$0x90, %ebp
	adc	$0x90, %esi
	adc	$0x90, %edi
	/* 0x83 */
	addb	$0x90, %eax
	addb	$0x90, %ecx
	addb	$0x90, %edx
	addb	$0x90, %ebx
	addb	$0x90, %esp
	addb	$0x90, %ebp
	addb	$0x90, %esi
	addb	$0x90, %edi
	/* 0x90 */
	wait
	fwait
	/* 0xa0 */
	stosd
	scasd
	cmpsb
	/* 0xc0 */
	ret
	/* 0xd4 */
	aam
	/* 0xd5 */
	aad
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
	/* 0xd8e0 */
	fsub	%st0, %st0, %st0
	fsub	%st0, %st1, %st0
	fsub	%st0, %st2, %st0
	fsub	%st0, %st3, %st0
	fsub	%st0, %st4, %st0
	fsub	%st0, %st5, %st0
	fsub	%st0, %st6, %st0
	fsub	%st0, %st7, %st0
	fsubr	%st0, %st0
	fsubr	%st1, %st0
	fsubr	%st2, %st0
	fsubr	%st3, %st0
	fsubr	%st4, %st0
	fsubr	%st5, %st0
	fsubr	%st6, %st0
	fsubr	%st7, %st0
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
	/* 0xd9c0 */
	fxch	%st0, %st0
	fxch	%st0, %st1
	fxch	%st0, %st2
	fxch	%st0, %st3
	fxch	%st0, %st4
	fxch	%st0, %st5
	fxch	%st0, %st6
	fxch	%st0, %st7
	/* 0xd9d0 */
	fnop
	/* 0xd9e0 */
	fchs
	fabs
	ftst
	fxam	%st0
	fldl2e
	fldpi
	fldlg2
	fldln2
	fldz
	f2xm1
	fyl2x	%st0, %st1, %st0
	fptan
	fxtract	%st0
	fprem1
	fdecstp
	fprem
	fyl2xp1	%st0, %st1, %st0
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
	fucompp	%st0, %st1
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
	/* 0xdce0 */
	fsubr	%st0, %st0
	fsubr	%st0, %st1
	fsubr	%st0, %st2
	fsubr	%st0, %st3
	fsubr	%st0, %st4
	fsubr	%st0, %st5
	fsubr	%st0, %st6
	fsubr	%st0, %st7
	fsub	%st0, %st0, %st0
	fsub	%st1, %st0, %st1
	fsub	%st2, %st0, %st2
	fsub	%st3, %st0, %st3
	fsub	%st4, %st0, %st4
	fsub	%st5, %st0, %st5
	fsub	%st6, %st0, %st6
	fsub	%st7, %st0, %st7
	/* 0xdd */
	/* 0xddd0 */
	fst	%st0, %st0
	fst	%st0, %st1
	fst	%st0, %st2
	fst	%st0, %st3
	fst	%st0, %st4
	fst	%st0, %st5
	fst	%st0, %st6
	fst	%st0, %st7
	fstp	%st0, %st0
	fstp	%st0, %st1
	fstp	%st0, %st2
	fstp	%st0, %st3
	fstp	%st0, %st4
	fstp	%st0, %st5
	fstp	%st0, %st6
	fstp	%st0, %st7
	/* 0xdde0 */
	fucom	%st0, %st0
	fucom	%st0, %st1
	fucom	%st0, %st2
	fucom	%st0, %st3
	fucom	%st0, %st4
	fucom	%st0, %st5
	fucom	%st0, %st6
	fucom	%st0, %st7
	fucomp	%st0, %st0
	fucomp	%st0, %st1
	fucomp	%st0, %st2
	fucomp	%st0, %st3
	fucomp	%st0, %st4
	fucomp	%st0, %st5
	fucomp	%st0, %st6
	fucomp	%st0, %st7
	/* 0xde */
	fcompp	%st0, %st1
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
	fsubrp	%st0, %st0
	fsubrp	%st0, %st1
	fsubrp	%st0, %st2
	fsubrp	%st0, %st3
	fsubrp	%st0, %st4
	fsubrp	%st0, %st5
	fsubrp	%st0, %st6
	fsubrp	%st0, %st7
	fsubp	%st0, %st0, %st0
	fsubp	%st1, %st0, %st1
	fsubp	%st2, %st0, %st2
	fsubp	%st3, %st0, %st3
	fsubp	%st4, %st0, %st4
	fsubp	%st5, %st0, %st5
	fsubp	%st6, %st0, %st6
	fsubp	%st7, %st0, %st7
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

.main
	/* 0x00 */
	add	%al, %al
	add	%al, %cl
	add	%al, %dl
	add	%al, %bl
	add	%al, %ah
	add	%al, %ch
	add	%al, %dh
	add	%al, %bh
	add	%cl, %al
	add	%cl, %cl
	add	%cl, %dl
	add	%cl, %bl
	add	%cl, %ah
	add	%cl, %ch
	add	%cl, %dh
	add	%cl, %bh
	add	%dl, %al
	add	%dl, %cl
	add	%dl, %dl
	add	%dl, %bl
	add	%dl, %ah
	add	%dl, %ch
	add	%dl, %dh
	add	%dl, %bh
	add	%bl, %al
	add	%bl, %cl
	add	%bl, %dl
	add	%bl, %bl
	add	%bl, %ah
	add	%bl, %ch
	add	%bl, %dh
	add	%bl, %bh
	/* 0x00e0 */
	add	%ah, %al
	add	%ah, %cl
	add	%ah, %dl
	add	%ah, %bl
	add	%ah, %ah
	add	%ah, %ch
	add	%ah, %dh
	add	%ah, %bh
	add	%ch, %al
	add	%ch, %cl
	add	%ch, %dl
	add	%ch, %bl
	add	%ch, %ah
	add	%ch, %ch
	add	%ch, %dh
	add	%ch, %bh
	add	%dh, %al
	add	%dh, %cl
	add	%dh, %dl
	add	%dh, %bl
	add	%dh, %ah
	add	%dh, %ch
	add	%dh, %dh
	add	%dh, %bh
	add	%bh, %al
	add	%bh, %cl
	add	%bh, %dl
	add	%bh, %bl
	add	%bh, %ah
	add	%bh, %ch
	add	%bh, %dh
	add	%bh, %bh
	/* 0x01 */
	add	%ax, %ax
	add	%ax, %cx
	add	%ax, %dx
	add	%ax, %bx
	add	%ax, %sp
	add	%ax, %bp
	add	%ax, %si
	add	%ax, %di
	/* 0x04 */
	add	$0x90, %al
	/* 0x05 */
	add	$0x90, %ax
	push	%es
	push	%cs
	push	%fs
	cpuid
	push	%gs
	/* 0x0f */
	/* 0x0fba */
	bt	$0x90, %ax
	bt	$0x90, %cx
	bt	$0x90, %dx
	bt	$0x90, %bx
	bt	$0x90, %sp
	bt	$0x90, %bp
	bt	$0x90, %si
	bt	$0x90, %di
	bts	$0x90, %ax
	bts	$0x90, %cx
	bts	$0x90, %dx
	bts	$0x90, %bx
	bts	$0x90, %sp
	bts	$0x90, %bp
	bts	$0x90, %si
	bts	$0x90, %di
	/* 0x0fbc */
	bsf	%ax, %ax
	bsf	%ax, %cx
	bsf	%ax, %dx
	bsf	%ax, %bx
	bsf	%ax, %sp
	bsf	%ax, %bp
	bsf	%ax, %si
	bsf	%ax, %di
	bsf	%cx, %ax
	bsf	%cx, %cx
	bsf	%cx, %dx
	bsf	%cx, %bx
	bsf	%cx, %sp
	bsf	%cx, %bp
	bsf	%cx, %si
	bsf	%cx, %di
	bsf	%dx, %ax
	bsf	%dx, %cx
	bsf	%dx, %dx
	bsf	%dx, %bx
	bsf	%dx, %sp
	bsf	%dx, %bp
	bsf	%dx, %si
	bsf	%dx, %di
	bsf	%bx, %ax
	bsf	%bx, %cx
	bsf	%bx, %dx
	bsf	%bx, %bx
	bsf	%bx, %sp
	bsf	%bx, %bp
	bsf	%bx, %si
	bsf	%bx, %di
	/* 0x0fbd */
	bsr	%ax, %ax
	bsr	%ax, %cx
	bsr	%ax, %dx
	bsr	%ax, %bx
	bsr	%ax, %sp
	bsr	%ax, %bp
	bsr	%ax, %si
	bsr	%ax, %di
	bsr	%cx, %ax
	bsr	%cx, %cx
	bsr	%cx, %dx
	bsr	%cx, %bx
	bsr	%cx, %sp
	bsr	%cx, %bp
	bsr	%cx, %si
	bsr	%cx, %di
	bsr	%dx, %ax
	bsr	%dx, %cx
	bsr	%dx, %dx
	bsr	%dx, %bx
	bsr	%dx, %sp
	bsr	%dx, %bp
	bsr	%dx, %si
	bsr	%dx, %di
	bsr	%bx, %ax
	bsr	%bx, %cx
	bsr	%bx, %dx
	bsr	%bx, %bx
	bsr	%bx, %sp
	bsr	%bx, %bp
	bsr	%bx, %si
	bsr	%bx, %di
	/* 0x0ff0 */
	btr	$0x90, %ax
	btr	$0x90, %cx
	btr	$0x90, %dx
	btr	$0x90, %bx
	btr	$0x90, %sp
	btr	$0x90, %bp
	btr	$0x90, %si
	btr	$0x90, %di
	btc	$0x90, %ax
	btc	$0x90, %cx
	btc	$0x90, %dx
	btc	$0x90, %bx
	btc	$0x90, %sp
	btc	$0x90, %bp
	btc	$0x90, %si
	btc	$0x90, %di
	/* 0x10 */
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
	/* 0x16 */
	push	%ss
	/* 0x1e */
	push	%ds
	/* 0x11 */
	/* 0x11c0 */
	adc	%ax, %ax
	adc	%ax, %cx
	adc	%ax, %dx
	adc	%ax, %bx
	adc	%ax, %sp
	adc	%ax, %bp
	adc	%ax, %si
	adc	%ax, %di
	adc	%cx, %ax
	adc	%cx, %cx
	adc	%cx, %dx
	adc	%cx, %bx
	adc	%cx, %sp
	adc	%cx, %bp
	adc	%cx, %si
	adc	%cx, %di
	/* 0x11d0 */
	adc	%dx, %ax
	adc	%dx, %cx
	adc	%dx, %dx
	adc	%dx, %bx
	adc	%dx, %sp
	adc	%dx, %bp
	adc	%dx, %si
	adc	%dx, %di
	adc	%bx, %ax
	adc	%bx, %cx
	adc	%bx, %dx
	adc	%bx, %bx
	adc	%bx, %sp
	adc	%bx, %bp
	adc	%bx, %si
	adc	%bx, %di
	/* 0x20 */
	daa
	das
	/* 0x30 */
	aaa
	/* 0x50 */
	push	%ax
	push	%cx
	push	%dx
	push	%bx
	push	%sp
	push	%bp
	/* 0x60 */
	pusha
	/* 0x62 */
	/* 0x62c0 */
	bound	%ax, %ax
	/* 0x63 */
	/* 0x63c0 */
	arpl	%ax, %ax
	arpl	%ax, %cx
	arpl	%ax, %dx
	arpl	%ax, %bx
	arpl	%ax, %sp
	arpl	%ax, %bp
	arpl	%ax, %si
	arpl	%ax, %di
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
	/* 0x80e0 */
	and	$0x90, %al
	and	$0x90, %cl
	and	$0x90, %dl
	and	$0x90, %bl
	and	$0x90, %ah
	and	$0x90, %ch
	and	$0x90, %dh
	and	$0x90, %bh
	/* 0x81 */
	add	$0x90, %ax
	add	$0x90, %cx
	add	$0x90, %dx
	add	$0x90, %bx
	add	$0x90, %sp
	add	$0x90, %bp
	add	$0x90, %si
	add	$0x90, %di
	adc	$0x90, %ax
	adc	$0x90, %cx
	adc	$0x90, %dx
	adc	$0x90, %bx
	adc	$0x90, %sp
	adc	$0x90, %bp
	adc	$0x90, %si
	adc	$0x90, %di
	/* 0x81e0 */
	and	$0x90, %ax
	and	$0x90, %cx
	and	$0x90, %dx
	and	$0x90, %bx
	and	$0x90, %sp
	and	$0x90, %bp
	and	$0x90, %si
	and	$0x90, %di
	/* 0x83 */
	addb	$0x90, %ax
	addb	$0x90, %cx
	addb	$0x90, %dx
	addb	$0x90, %bx
	addb	$0x90, %sp
	addb	$0x90, %bp
	addb	$0x90, %si
	addb	$0x90, %di
	/* 0x83e0 */
	andb	$0x90, %ax
	andb	$0x90, %cx
	andb	$0x90, %dx
	andb	$0x90, %bx
	andb	$0x90, %sp
	andb	$0x90, %bp
	andb	$0x90, %si
	andb	$0x90, %di
	/* 0x8d */
	lea	[%si], %ax
	lea	[%di], %ax
	lea	[%bx], %ax
	/* 0x90 */
	cbw	%ax
	wait
	fwait
	/* 0xa0 */
	stosw
	scasw
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
	fyl2x	%st1, %st0, %st1
	fptan
	fxtract	%st0
	fprem1
	fdecstp
	fprem
	fyl2xp1	%st1, %st0, %st1
	fsqrt	%st0
	fsincos	%st0
	frndint
	fscale	%st0, %st1, %st0
	fsin	%st0
	fcos	%st0
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
	/* 0xe0 */
	call	$0x90
	/* 0xf0 */
	cmc
	clc
	stc
	cli
	sti
	cld
	std

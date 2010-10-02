.text
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
	add	%eax, %eax
	add	%eax, %ecx
	add	%eax, %edx
	add	%eax, %ebx
	add	%eax, %esp
	add	%eax, %ebp
	add	%eax, %esi
	add	%eax, %edi
	/* 0x04 */
	add	$0x90, %al
	/* 0x05 */
	add	$0x90, %eax
	push	%es
	push	%cs
	push	%fs
	cpuid
	push	%gs
	/* 0x09 */
	lahf
	/* 0x0f */
	/* 0x0f00 */
	lldt	[%eax]
	lldt	[%ecx]
	lldt	[%edx]
	lldt	[%ebx]
	lldt	[%esi]
	lldt	[%edi]
	lldt	%eax
	lldt	%ecx
	lldt	%edx
	lldt	%ebx
	lldt	%esp
	lldt	%ebp
	lldt	%esi
	lldt	%edi
	/* 0x0f09 */
	wbinvd
	ud2
	/* 0x0f30 */
	wrmsr
	/* 0x0fb1 */
	cmpxchg	%eax, %eax
	cmpxchg	%eax, %ecx
	cmpxchg	%eax, %edx
	cmpxchg	%eax, %ebx
	cmpxchg	%eax, %esp
	cmpxchg	%eax, %ebp
	cmpxchg	%eax, %esi
	cmpxchg	%eax, %edi
	/* 0x0fba */
	bt	$0x90, %eax
	bt	$0x90, %ecx
	bt	$0x90, %edx
	bt	$0x90, %ebx
	bt	$0x90, %esp
	bt	$0x90, %ebp
	bt	$0x90, %esi
	bt	$0x90, %edi
	bts	$0x90, %eax
	bts	$0x90, %ecx
	bts	$0x90, %edx
	bts	$0x90, %ebx
	bts	$0x90, %esp
	bts	$0x90, %ebp
	bts	$0x90, %esi
	bts	$0x90, %edi
	/* 0x0fbc */
	bsf	%eax, %eax
	bsf	%eax, %ecx
	bsf	%eax, %edx
	bsf	%eax, %ebx
	bsf	%eax, %esp
	bsf	%eax, %ebp
	bsf	%eax, %esi
	bsf	%eax, %edi
	bsf	%ecx, %eax
	bsf	%ecx, %ecx
	bsf	%ecx, %edx
	bsf	%ecx, %ebx
	bsf	%ecx, %esp
	bsf	%ecx, %ebp
	bsf	%ecx, %esi
	bsf	%ecx, %edi
	bsf	%edx, %eax
	bsf	%edx, %ecx
	bsf	%edx, %edx
	bsf	%edx, %ebx
	bsf	%edx, %esp
	bsf	%edx, %ebp
	bsf	%edx, %esi
	bsf	%edx, %edi
	bsf	%ebx, %eax
	bsf	%ebx, %ecx
	bsf	%ebx, %edx
	bsf	%ebx, %ebx
	bsf	%ebx, %esp
	bsf	%ebx, %ebp
	bsf	%ebx, %esi
	bsf	%ebx, %edi
	/* 0x0fbd */
	bsr	%eax, %eax
	bsr	%eax, %ecx
	bsr	%eax, %edx
	bsr	%eax, %ebx
	bsr	%eax, %esp
	bsr	%eax, %ebp
	bsr	%eax, %esi
	bsr	%eax, %edi
	bsr	%ecx, %eax
	bsr	%ecx, %ecx
	bsr	%ecx, %edx
	bsr	%ecx, %ebx
	bsr	%ecx, %esp
	bsr	%ecx, %ebp
	bsr	%ecx, %esi
	bsr	%ecx, %edi
	bsr	%edx, %eax
	bsr	%edx, %ecx
	bsr	%edx, %edx
	bsr	%edx, %ebx
	bsr	%edx, %esp
	bsr	%edx, %ebp
	bsr	%edx, %esi
	bsr	%edx, %edi
	bsr	%ebx, %eax
	bsr	%ebx, %ecx
	bsr	%ebx, %edx
	bsr	%ebx, %ebx
	bsr	%ebx, %esp
	bsr	%ebx, %ebp
	bsr	%ebx, %esi
	bsr	%ebx, %edi
	/* 0x0ff0 */
	btr	$0x90, %eax
	btr	$0x90, %ecx
	btr	$0x90, %edx
	btr	$0x90, %ebx
	btr	$0x90, %esp
	btr	$0x90, %ebp
	btr	$0x90, %esi
	btr	$0x90, %edi
	btc	$0x90, %eax
	btc	$0x90, %ecx
	btc	$0x90, %edx
	btc	$0x90, %ebx
	btc	$0x90, %esp
	btc	$0x90, %ebp
	btc	$0x90, %esi
	btc	$0x90, %edi
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
	/* 0x28 */
	sub	%al, %al
	sub	%eax, %eax
	/* 0x2c */
	sub	$0x90, %al
	sub	$0x90, %eax
	/* 0x2f */
	das
	/* 0x30 */
	xor	%al, %al
	xor	%al, %cl
	xor	%al, %dl
	xor	%al, %bl
	xor	%al, %ah
	xor	%al, %ch
	xor	%al, %dh
	xor	%al, %bh
	/* 0x31 */
	xor	%eax, %eax
	xor	%eax, %ecx
	xor	%eax, %edx
	xor	%eax, %ebx
	xor	%eax, %esp
	xor	%eax, %ebp
	xor	%eax, %esi
	xor	%eax, %edi
	/* 0x32 */
	xor	%al, %al
	xor	%cl, %al
	xor	%dl, %al
	xor	%bl, %al
	xor	%ah, %al
	xor	%ch, %al
	xor	%dh, %al
	xor	%bh, %al
	/* 0x33 */
	xor	%eax, %eax
	xor	%ecx, %eax
	xor	%edx, %eax
	xor	%ebx, %eax
	xor	%esp, %eax
	xor	%ebp, %eax
	xor	%esi, %eax
	xor	%edi, %eax
	/* 0x37 */
	aaa
	/* 0x38 */
	cmp	%al, %al
	cmp	%al, %cl
	cmp	%al, %dl
	cmp	%al, %bl
	cmp	%al, %ah
	cmp	%al, %ch
	cmp	%al, %dh
	cmp	%al, %bh
	/* 0x39 */
	cmp	%eax, %eax
	cmp	%eax, %ecx
	cmp	%eax, %edx
	cmp	%eax, %ebx
	cmp	%eax, %esp
	cmp	%eax, %ebp
	cmp	%eax, %esi
	cmp	%eax, %edi
	/* 0x40 */
	inc	%eax
	inc	%ecx
	inc	%edx
	inc	%ebx
	inc	%esp
	inc	%ebp
	inc	%esi
	inc	%edi
	dec	%eax
	dec	%ecx
	dec	%edx
	dec	%ebx
	dec	%esp
	dec	%ebp
	dec	%esi
	dec	%edi
	/* 0x50 */
	push	%eax
	push	%ecx
	push	%edx
	push	%ebx
	push	%esp
	push	%ebp
	push	%esi
	push	%edi
	/* 0x58 */
	pop	%eax
	pop	%ecx
	pop	%edx
	pop	%ebx
	pop	%esp
	pop	%ebp
	pop	%esi
	pop	%edi
	/* 0x60 */
	pusha
	/* 0x62 */
	/* 0x62c0 */
	bound	%eax, %eax
	/* 0x63 */
	/* 0x63c0 */
	arpl	%eax, %eax
	arpl	%eax, %ecx
	arpl	%eax, %edx
	arpl	%eax, %ebx
	arpl	%eax, %esp
	arpl	%eax, %ebp
	arpl	%eax, %esi
	arpl	%eax, %edi
	/* 0x80 */
	add	$0x90, %al
	add	$0x90, %cl
	add	$0x90, %dl
	add	$0x90, %bl
	add	$0x90, %ah
	add	$0x90, %ch
	add	$0x90, %dh
	add	$0x90, %bh
	adc	$0x90, [%eax]
	adc	$0x90, [%ecx]
	adc	$0x90, [%edx]
	adc	$0x90, [%ebx]
	adc	$0x90, [%esi]
	adc	$0x90, [%edi]
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
	/* 0x80f0 */
	cmp	$0x90, %al
	cmp	$0x90, %cl
	cmp	$0x90, %dl
	cmp	$0x90, %bl
	cmp	$0x90, %ah
	cmp	$0x90, %ch
	cmp	$0x90, %dh
	cmp	$0x90, %bh
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
	/* 0x81e0 */
	and	$0x90, %eax
	and	$0x90, %ecx
	and	$0x90, %edx
	and	$0x90, %ebx
	and	$0x90, %esp
	and	$0x90, %ebp
	and	$0x90, %esi
	and	$0x90, %edi
	/* 0x81f0 */
	cmp	$0x90, %eax
	cmp	$0x90, %ecx
	cmp	$0x90, %edx
	cmp	$0x90, %ebx
	cmp	$0x90, %esp
	cmp	$0x90, %ebp
	cmp	$0x90, %esi
	cmp	$0x90, %edi
	/* 0x83 */
	addb	$0x90, %eax
	addb	$0x90, %ecx
	addb	$0x90, %edx
	addb	$0x90, %ebx
	addb	$0x90, %esp
	addb	$0x90, %ebp
	addb	$0x90, %esi
	addb	$0x90, %edi
	/* 0x83e0 */
	andb	$0x90, %eax
	andb	$0x90, %ecx
	andb	$0x90, %edx
	andb	$0x90, %ebx
	andb	$0x90, %esp
	andb	$0x90, %ebp
	andb	$0x90, %esi
	andb	$0x90, %edi
	/* 0x86 */
	xchg	%al, %al
	xchg	%al, %cl
	xchg	%al, %dl
	xchg	%al, %bl
	xchg	%al, %ah
	xchg	%al, %ch
	xchg	%al, %dh
	xchg	%al, %bh
	/* 0x89 */
	mov	%esp, %ebp
	/* 0x8d */
	lea	[%eax], %eax
	lea	[%ecx], %eax
	lea	[%edx], %eax
	lea	[%ebx], %eax
	lea	[%esi], %eax
	lea	[%edi], %eax
	/* 0x8f */
	pop	[%eax]
	pop	[%ecx]
	pop	[%edx]
	pop	[%ebx]
	pop	[%esi]
	pop	[%edi]
	pop	%eax
	pop	%ecx
	pop	%edx
	pop	%ebx
	pop	%esp
	pop	%ebp
	pop	%esi
	pop	%edi
	/* 0x90 */
	nop
	xchg	%eax, %eax
	xchg	%eax, %ecx
	xchg	%eax, %edx
	xchg	%eax, %ebx
	xchg	%eax, %esp
	xchg	%eax, %ebp
	xchg	%eax, %esi
	xchg	%eax, %edi
	cwde	%eax
	cdq
	wait
	fwait
	/* 0xa0 */
	cmpsb
	stosd
	/* 0xa8 */
	test	$0x90, %al
	test	$0x90, %eax
	/* 0xad */
	lodsd
	scasd
	/* 0xc0 */
	ret
	/* 0xc8 */
	enter	$0x90, $0x90
	int3
	int	$0x90
	into
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
	fmul	%st0, %st0, %st0
	fmul	%st0, %st1, %st0
	fmul	%st0, %st2, %st0
	fmul	%st0, %st3, %st0
	fmul	%st0, %st4, %st0
	fmul	%st0, %st5, %st0
	fmul	%st0, %st6, %st0
	fmul	%st0, %st7, %st0
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
	fld	%st0
	fld	%st1
	fld	%st2
	fld	%st3
	fld	%st4
	fld	%st5
	fld	%st6
	fld	%st7
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
	fpatan
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
	fmul	%st0, %st0, %st0
	fmul	%st1, %st0, %st1
	fmul	%st2, %st0, %st2
	fmul	%st3, %st0, %st3
	fmul	%st4, %st0, %st4
	fmul	%st5, %st0, %st5
	fmul	%st6, %st0, %st6
	fmul	%st7, %st0, %st7
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
	/* 0xddc0 */
	ffree	%st0
	ffree	%st1
	ffree	%st2
	ffree	%st3
	ffree	%st4
	ffree	%st5
	ffree	%st6
	ffree	%st7
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
	fmulp	%st0, %st0, %st0
	fmulp	%st1, %st0, %st1
	fmulp	%st2, %st0, %st2
	fmulp	%st3, %st0, %st3
	fmulp	%st4, %st0, %st4
	fmulp	%st5, %st0, %st5
	fmulp	%st6, %st0, %st6
	fmulp	%st7, %st0, %st7
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
	/* 0xdf */
	/* 0xdfd0 */
	fist	%st0, %ax
	fist	%st0, %cx
	fist	%st0, %dx
	fist	%st0, %bx
	fist	%st0, %sp
	fist	%st0, %bp
	fist	%st0, %si
	fist	%st0, %di
	fistp	%st0, %ax
	fistp	%st0, %cx
	fistp	%st0, %dx
	fistp	%st0, %bx
	fistp	%st0, %sp
	fistp	%st0, %bp
	fistp	%st0, %si
	fistp	%st0, %di
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
	/* 0xe4 */
	in	$0x90, %al
	in	$0x90, %eax
	/* 0xe8 */
	call	$0x90
	in	%dx, %al
	in	%dx, %eax
	/* 0xf0 */
	cmc
	/* 0xf6 */
	div	%ax, %al, %al
	div	%ax, %cl, %al
	div	%ax, %dl, %al
	div	%ax, %bl, %al
	div	%ax, %ah, %al
	div	%ax, %ch, %al
	div	%ax, %dh, %al
	div	%ax, %bh, %al
	/* 0xf6f0 */
	idiv	%ax, %al, %al
	idiv	%ax, %cl, %al
	idiv	%ax, %dl, %al
	idiv	%ax, %bl, %al
	idiv	%ax, %ah, %al
	idiv	%ax, %ch, %al
	idiv	%ax, %dh, %al
	idiv	%ax, %bh, %al
	/* 0xf7 */
	div	%eax
	div	%ecx
	div	%edx
	div	%ebx
	div	%esp
	div	%ebp
	div	%esi
	div	%edi
	/* 0xf7f0 */
	idiv	%eax, %eax, %eax
	idiv	%eax, %ecx, %eax
	idiv	%eax, %edx, %eax
	idiv	%eax, %ebx, %eax
	idiv	%eax, %esp, %eax
	idiv	%eax, %ebp, %eax
	idiv	%eax, %esi, %eax
	idiv	%eax, %edi, %eax
	/* 0xf8 */
	clc
	/* 0xf9 */
	stc
	/* 0xfa */
	cli
	sti
	cld
	std
	/* 0xfe */
	/* 0xfec0 */
	inc	%al
	inc	%cl
	inc	%dl
	inc	%bl
	inc	%ah
	inc	%ch
	inc	%dh
	inc	%bh
	dec	%al
	dec	%cl
	dec	%dl
	dec	%bl
	dec	%ah
	dec	%ch
	dec	%dh
	dec	%bh
	/* 0xff */
	/* 0xff20 */
	jmp	[%eax]
	jmp	[%ecx]
	jmp	[%edx]
	jmp	[%ebx]
	jmp	[%esi]
	jmp	[%edi]
	/* 0xffc0 */
	inc	%eax
	inc	%ecx
	inc	%edx
	inc	%ebx
	inc	%esp
	inc	%ebp
	inc	%esi
	inc	%edi
	dec	%eax
	dec	%ecx
	dec	%edx
	dec	%ebx
	dec	%esp
	dec	%ebp
	dec	%esi
	dec	%edi
	/* 0xffe0 */
	jmp	%eax
	jmp	%ecx
	jmp	%edx
	jmp	%ebx
	jmp	%esp
	jmp	%ebp
	jmp	%esi
	jmp	%edi
	/* db */
	db	$0x01

.main
	/* 0x0f */
	/* 0x0f00 */
	invlpg	%eax
	invlpg	%ecx
	invlpg	%edx
	invlpg	%ebx
	invlpg	%esp
	invlpg	%ebp
	invlpg	%esi
	invlpg	%edi
	/* 0x0fc0 */
	bswap	%eax
	bswap	%ecx
	bswap	%edx
	bswap	%ebx
	bswap	%esp
	bswap	%ebp
	bswap	%esi
	bswap	%edi

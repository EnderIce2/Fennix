[bits 64]

[global _i386_fxsave]
_i386_fxsave:
	fxsave [edi]
	ret

[global _i386_fxrstor]
_i386_fxrstor:
	fxrstor [edi]
	ret

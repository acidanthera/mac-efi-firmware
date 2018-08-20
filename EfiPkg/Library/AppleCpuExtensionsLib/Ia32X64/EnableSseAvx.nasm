    DEFAULT REL
    SECTION .text

;------------------------------------------------------------------------------
; // InternalAsmEnableSseAvx
; VOID
; EFIAPI
; InternalAsmEnableSseAvx (
;   VOID
;   );
;------------------------------------------------------------------------------
global ASM_PFX(InternalAsmEnableSseAvx)
ASM_PFX(InternalAsmEnableSseAvx):
    xor     ecx, ecx
    xgetbv
    or      eax, 6
    xor     ecx, ecx
    xsetbv
    ret

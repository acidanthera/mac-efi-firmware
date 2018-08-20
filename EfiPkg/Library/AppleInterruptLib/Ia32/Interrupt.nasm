    DEFAULT REL
    SECTION .text

;------------------------------------------------------------------------------
; // AppleInterrupt
; VOID
; EFIAPI
; AppleInterrupt (
;   IN UINT32  FunctiondId,
;   ...
;   );
;------------------------------------------------------------------------------
global ASM_PFX(AppleInterrupt)
ASM_PFX(AppleInterrupt):
    push    ebp
    mov     ebp, esp
    int     21h
    pop     ebp
    retn

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
    push    rbp
    mov     rbp, rsp
    int     21h
    pop     rbp
    retn

#------------------------------------------------------------------------------
# // AppleInterrupt
# VOID
# EFIAPI
# AppleInterrupt (
#   IN UINT32  FunctiondId,
#   ...
#   );
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(AppleInterrupt)
ASM_PFX(AppleInterrupt):
    push    %rbp
    mov     %rbp, %rsp
    int     $0x21
    pop     %rbp
    ret

ASM_FUNCTION_REMOVE_IF_UNREFERENCED

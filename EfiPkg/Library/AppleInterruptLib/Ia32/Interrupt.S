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
    push    %ebp
    mov     %ebp, %esp
    int     $0x21
    pop     %ebp
    ret

ASM_FUNCTION_REMOVE_IF_UNREFERENCED

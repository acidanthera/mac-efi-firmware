#------------------------------------------------------------------------------
# // CheckStackCanary
# VOID
# EFIAPI
# CheckStackCanary (
#   IN UINTN  Value
#   );
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(CheckStackCanary)
ASM_PFX(CheckStackCanary):
    cmp     ASM_PFX(__stack_chk_guard), %ecx
    jnz     ASM_PFX(__stack_chk_fail)
    ret

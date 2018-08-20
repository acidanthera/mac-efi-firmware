#------------------------------------------------------------------------------
# // _ModuleEntryPoint
# <_ModuleEntryPointReturnType>
# EFIAPI
# _ModuleEntryPoint (
#   <_ModuleEntryPointArguments>
#   );
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(_ModuleEntryPoint)
ASM_PFX(_ModuleEntryPoint):
    push     %ebx
    lea      ASM_PFX(__stack_chk_guard), %ebx
    # rdrand   eax
    .byte    0x0F, 0xC7, 0xF0
    mov      %eax, (%ebx)
    pop      %ebx
    jmp      ASM_PFX(EfiMain)

ASM_FUNCTION_REMOVE_IF_UNREFERENCED

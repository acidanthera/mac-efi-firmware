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
    push     %rbx
    lea      ASM_PFX(__stack_chk_guard)(%rip), %rbx
    # rdrand   rax
    .byte    0x48, 0x0F, 0xC7, 0xF0
    mov      %rax, (%rbx)
    pop      %rbx
    jmp      ASM_PFX(EfiMain)

ASM_FUNCTION_REMOVE_IF_UNREFERENCED

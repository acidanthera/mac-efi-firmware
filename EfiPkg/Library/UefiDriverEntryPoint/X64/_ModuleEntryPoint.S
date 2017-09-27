#------------------------------------------------------------------------------
#
# Copyright (c) 2005 - 2017, Apple Inc.  All rights reserved.<BR>
#
# This program and the accompanying materials have not been licensed.
# Neither is its usage, its redistribution, in source or binary form,
# licensed, nor implicitely or explicitely permitted, except when
# required by applicable law.
#
# Unless required by applicable law or agreed to in writing, software
# distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#
#------------------------------------------------------------------------------

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

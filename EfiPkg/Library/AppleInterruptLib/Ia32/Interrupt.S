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

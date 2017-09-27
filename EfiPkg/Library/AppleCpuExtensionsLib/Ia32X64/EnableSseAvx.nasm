;------------------------------------------------------------------------------
;
; Copyright (c) 2005 - 2017, Apple Inc.  All rights reserved.<BR>
;
; This program and the accompanying materials have not been licensed.
; Neither is its usage, its redistribution, in source or binary form,
; licensed, nor implicitely or explicitely permitted, except when
; required by applicable law.
;
; Unless required by applicable law or agreed to in writing, software
; distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
; OR CONDITIONS OF ANY KIND, either express or implied.
;
;------------------------------------------------------------------------------

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

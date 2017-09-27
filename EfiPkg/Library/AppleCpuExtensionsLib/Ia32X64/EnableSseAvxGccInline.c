/** @file
  Copyright (c) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <Base.h>

// InternalAsmEnableSseAvx
VOID
InternalAsmEnableSseAvx (
  VOID
  )
{
  __asm__ __volatile__ (
    "xor     %ecx, %ecx\n\t"
    // xgetbv
    ".byte   0x0F, 0x01, 0xD0\n\t"
    "orl     $0x06, %eax\n\t"
    "xor     %ecx, %ecx\n\t"
    // xsetbv
    ".byte   0x0F, 0x01, 0xD1"
    );
}

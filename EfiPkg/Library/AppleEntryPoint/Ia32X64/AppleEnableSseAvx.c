/** @file
  Copyright (c) 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleMacEfi.h>

#include <Library/BaseLib.h>

// InternalAsmEnableSseAvx
VOID
EFIAPI
InternalAsmEnableSseAvx (
  VOID
  );

// AppleEnableSseAvx
VOID
AppleEnableSseAvx (
  VOID
  )
{
  UINTN Cr4;

  Cr4 = AsmReadCr4 ();

  //
  // check CR4.OSXSAVE[Bit 18]
  //
  if ((Cr4 & BIT18) != 0) {
    //
    // set CR4.OSXSAVE[Bit 18]
    //
    AsmWriteCr4 (Cr4 | BIT18);

    //
    //  enable SSE and AVX
    //
    InternalAsmEnableSseAvx ();
  }
}

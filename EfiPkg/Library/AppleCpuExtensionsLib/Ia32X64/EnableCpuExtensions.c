#include <Base.h>

#include <Library/AppleCpuExtensionsLib.h>
#include <Library/BaseLib.h>

// InternalAsmEnableSseAvx
VOID
EFIAPI
InternalAsmEnableSseAvx (
  VOID
  );

// AppleEnableCpuExtensions
VOID
AppleEnableCpuExtensions (
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
    // enable SSE and AVX
    //
    InternalAsmEnableSseAvx ();
  }
}

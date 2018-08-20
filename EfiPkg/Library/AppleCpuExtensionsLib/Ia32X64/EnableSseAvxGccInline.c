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

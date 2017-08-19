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

#include <AppleMacEfi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#include <IndustryStandard/AppleSmBios.h>

/**
  CPUID Processor Brand String

  @param   EAX  CPUID_BRAND_STRING1 (0x80000002)

  @retval  EAX  Processor Brand String in type CPUID_BRAND_STRING_DATA.
  @retval  EBX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  ECX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  EDX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.

  <b>Example usage</b>
  @code
  CPUID_BRAND_STRING_DATA  Eax;
  CPUID_BRAND_STRING_DATA  Ebx;
  CPUID_BRAND_STRING_DATA  Ecx;
  CPUID_BRAND_STRING_DATA  Edx;

  AsmCpuid (CPUID_BRAND_STRING1, &Eax.Uint32, &Ebx.Uint32, &Ecx.Uint32, &Edx.Uint32);
  @endcode
**/
#define CPUID_BRAND_STRING1 0x80000002

/**
  CPUID Processor Brand String

  @param   EAX  CPUID_BRAND_STRING2 (0x80000003)

  @retval  EAX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  EBX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  ECX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  EDX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.

  <b>Example usage</b>
  @code
  CPUID_BRAND_STRING_DATA  Eax;
  CPUID_BRAND_STRING_DATA  Ebx;
  CPUID_BRAND_STRING_DATA  Ecx;
  CPUID_BRAND_STRING_DATA  Edx;

  AsmCpuid (CPUID_BRAND_STRING2, &Eax.Uint32, &Ebx.Uint32, &Ecx.Uint32, &Edx.Uint32);
  @endcode
**/
#define CPUID_BRAND_STRING2 0x80000003

/**
  CPUID Processor Brand String

  @param   EAX  CPUID_BRAND_STRING3 (0x80000004)

  @retval  EAX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  EBX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  ECX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.
  @retval  EDX  Processor Brand String Continued in type CPUID_BRAND_STRING_DATA.

  <b>Example usage</b>
  @code
  CPUID_BRAND_STRING_DATA  Eax;
  CPUID_BRAND_STRING_DATA  Ebx;
  CPUID_BRAND_STRING_DATA  Ecx;
  CPUID_BRAND_STRING_DATA  Edx;

  AsmCpuid (CPUID_BRAND_STRING3, &Eax.Uint32, &Ebx.Uint32, &Ecx.Uint32, &Edx.Uint32);
  @endcode
**/
#define CPUID_BRAND_STRING3 0x80000004

// CPUID_BRAND_STRING
typedef struct {
  UINT32 Eax;
  UINT32 Ebx;
  UINT32 Ecx;
  UINT32 Edx;
} CPUID_BRAND_STRING;

// InternalGetProcessorClass
VOID
InternalGetProcessorClass (
  OUT UINT8  *ProcessorClass
  )
{
  CHAR8              BrandString[3 * sizeof (CPUID_BRAND_STRING) + 1];
  UINT32             Index;
  CPUID_BRAND_STRING BrandStringBuffer;
  CHAR8              *BrandStringWalker;

  BrandString[ARRAY_SIZE (BrandString) - 1] = '\0';

  BrandStringWalker = &BrandString[0];

  for (Index = CPUID_BRAND_STRING1; Index != CPUID_BRAND_STRING3; ++Index) {
    AsmCpuid (
      Index,
      &BrandStringBuffer.Eax,
      &BrandStringBuffer.Ebx,
      &BrandStringBuffer.Ecx,
      &BrandStringBuffer.Edx
      );

    CopyMem (
      (VOID *)&BrandStringBuffer,
      (VOID *)BrandStringWalker,
      sizeof (BrandStringBuffer)
      );

    ++BrandStringWalker;
  }

  BrandStringWalker = AsciiStrStr (&BrandString[0], "Core");

  while ((*BrandStringWalker != ' ') && (*BrandStringWalker != '\0')) {
    ++BrandStringWalker;
  }

  while (*BrandStringWalker == ' ') {
    ++BrandStringWalker;
  }

  if (AsciiStrCmp (BrandStringWalker, "i7") == 0) {
    *ProcessorClass = AppleProcessorClassI7;
  } else if (AsciiStrCmp (BrandStringWalker, "i5") == 0) {
    *ProcessorClass= AppleProcessorClassI5;
  } else if (AsciiStrCmp (BrandStringWalker, "i3") == 0) {
    *ProcessorClass= AppleProcessorClassI3;
  } else if (AsciiStrCmp (BrandStringWalker, "m3") == 0) {
    *ProcessorClass= AppleProcessorClassM3;
  } else if (AsciiStrCmp (BrandStringWalker, "m5") == 0) {
    *ProcessorClass= AppleProcessorClassM5;
  } else if (AsciiStrCmp (BrandStringWalker, "m7") == 0) {
    *ProcessorClass= AppleProcessorClassM7;
  }
}

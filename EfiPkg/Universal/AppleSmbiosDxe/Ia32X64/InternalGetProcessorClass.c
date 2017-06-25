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

#include <Register/Cpuid.h>

// CPUID_BRAND_STRING
typedef struct {
  CPUID_BRAND_STRING_DATA Eax;
  CPUID_BRAND_STRING_DATA Ebx;
  CPUID_BRAND_STRING_DATA Ecx;
  CPUID_BRAND_STRING_DATA Edx;
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
      &BrandStringBuffer.Eax.Uint32,
      &BrandStringBuffer.Ebx.Uint32,
      &BrandStringBuffer.Ecx.Uint32,
      &BrandStringBuffer.Edx.Uint32
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

/** @file
  Copyright (C) 2005 - 2015 Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include APPLE_GUID_DEFINITION (AppleOsLoaded)

#include <Library/AppleDriverLib.h>

#include "OsIdentificationImplInternal.h"

// OsIdentificationOSNameImpl
VOID
EFIAPI
OsIdentificationOSNameImpl (
  IN CHAR8  *OSName
  )
{
  ASSERT (OSName != NULL);

  return;
}

// OsIdentificationOSVendorImpl
VOID
EFIAPI
OsIdentificationOSVendorImpl (
  IN CHAR8  *OSVendor
  )
{
  INTN Result;

  ASSERT (OSVendor != NULL);

  Result = EfiAsciiStrCmp (OSVendor, OS_IDENTIFICATION_VENDOR_NAME);

  if (Result == 0) {
    EfiLibNamedEventSignal (&gAppleOsLoadedNamedEventGuid);
  }
}

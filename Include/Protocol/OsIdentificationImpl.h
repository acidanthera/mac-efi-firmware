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

#ifndef OS_IDENTIFICATION_IMPL_H_
#define OS_IDENTIFICATION_IMPL_H_

#include <Protocol/OsIdentification.h>

// OS_IDENTIFICATION_PROTOCOL_REVISION
#define OS_IDENTIFICATION_PROTOCOL_REVISION  0x01

// OS_IDENTIFICATION_VENDOR_NAME
#define OS_IDENTIFICATION_VENDOR_NAME  "Apple Inc."

// OsIdentificationOSNameImpl
VOID
EFIAPI
OsIdentificationOSNameImpl (
  IN CHAR8 *OSName
  );

// OsIdentificationOSVendorImpl
VOID
EFIAPI
OsIdentificationOSVendorImpl (
  IN CHAR8 *OSVendor
  );

#endif // OS_IDENTIFICATION_IMPL_H_

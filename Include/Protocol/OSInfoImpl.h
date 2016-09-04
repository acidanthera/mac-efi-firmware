/** @file
  Copyright (C) 2005 - 2015, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#ifndef OS_INFO_IMPL_H_
#define OS_INFO_IMPL_H_

#include APPLE_PROTOCOL_PRODUCER (OSInfo)

// OS_INFO_PROTOCOL_REVISION
#define EFI_OS_INFO_PROTOCOL_REVISION  0x01

// OS_INFO_VENDOR_NAME
#define OS_INFO_VENDOR_NAME  "Apple Inc."

// OSInfoOSNameImpl
VOID
EFIAPI
OSInfoOSNameImpl (
  IN CHAR8 *OSName
  );

// OSInfoOSVendorImpl
VOID
EFIAPI
OSInfoOSVendorImpl (
  IN CHAR8 *OSVendor
  );

#endif // OS_INFO_IMPL_H_

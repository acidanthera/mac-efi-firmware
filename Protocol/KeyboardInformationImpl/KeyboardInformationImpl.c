/** @file
  Copyright (C) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include APPLE_PROTOCOL_PRODUCER (KeyboardInformationImpl)

// gKeyboardInfoIdVendor
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 gKbInfoIdVendor = 0;

// gKbInfoCountryCode
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 gKbInfoCountryCode = 0;

// gKeyboardInfoIdProduct
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 gKbInfoIdProduct = 0;

// KbInfoGetInfo
EFI_STATUS
EFIAPI
KbInfoGetInfo (
  OUT UINT16  *IdVendor,
  OUT UINT16  *IdProduct,
  OUT UINT8   *CountryCode
  )
{
  ASSERT (IdVendor != NULL);
  ASSERT (IdProduct != NULL);
  ASSERT (CountryCode != NULL);

  *IdVendor    = gKbInfoIdVendor;
  *IdProduct   = gKbInfoIdProduct;
  *CountryCode = gKbInfoCountryCode;

  return EFI_SUCCESS;
}

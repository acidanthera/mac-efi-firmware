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

#include <AppleEfi.h>

#include APPLE_PROTOCOL_PRODUCER (KeyboardInformationImpl)

// gKeyboardInfoIdVendor
UINT16 gKbInfoIdVendor = 0;

// gKeyboardInfoCountryCode
UINT8 gKeyboardInfoCountryCode = 0;

// gKeyboardInfoIdProduct
UINT16 gKbInfoIdProduct = 0;

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
  ASSERT (CountryCode);

  *IdVendor    = gKbInfoIdVendor;
  *IdProduct   = gKbInfoIdProduct;
  *CountryCode = gKeyboardInfoCountryCode;

  return EFI_SUCCESS;
}

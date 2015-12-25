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

#ifndef KEYBOARD_INFORMATION_IMPL_H_
#define KEYBOARD_INFORMATION_IMPL_H_

#include <Protocol/KeyboardInfo.h>

// KbInfoGetInfo
EFI_STATUS
EFIAPI
KbInfoGetInfo (
  OUT UINT16  *IdVendor,
  OUT UINT16  *IdProduct,
  OUT UINT8   *CountryCode
  );

// gKeyboardInfoIdVendor
extern UINT16 gKeyboardInfoIdVendor;

// gKeyboardInfoCountryCode
extern UINT8 gKeyboardInfoCountryCode;

// gKeyboardInfoIdProduct
extern UINT16 gKeyboardInfoIdProduct;

#endif // KEYBOARD_INFORMATION_IMPL_H_


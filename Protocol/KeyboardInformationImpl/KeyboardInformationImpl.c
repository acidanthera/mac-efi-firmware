//
// Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
//
// This program and the accompanying materials have not been licensed.
// Neither is its usage, its redistribution, in source or binary form,
// licensed, nor implicitely or explicitely permitted, except when
// required by applicable law.
//
// Unless required by applicable law or agreed to in writing, software
// distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
// OR CONDITIONS OF ANY KIND, either express or implied.
//

///
/// @file      Protocol/KeyboardInformationImpl/KeyboardInformationImpl.c
///
/// @brief     
///
/// @author    Download-Fritz
/// @date      13/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015, Apple Inc
///

#include <AppleEfi.h>

#include <Protocol/KeyboardInformationImpl.h>

// gKeyboardInfoIdVendor
UINT16 gKeyboardInfoIdVendor = 0;

// gKeyboardInfoCountryCode
UINT8 gKeyboardInfoCountryCode = 0;

// gKeyboardInfoIdProduct
UINT16 gKeyboardInfoIdProduct = 0;

// KbInfoGetInfo
/// 
///
/// @param 
///
/// @return 
/// @retval
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

  *IdVendor    = gKeyboardInfoIdVendor;
  *IdProduct   = gKeyboardInfoIdProduct;
  *CountryCode = gKeyboardInfoCountryCode;

  return EFI_SUCCESS;
}

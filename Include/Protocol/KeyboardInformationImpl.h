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
/// @file      Include/Protocol/KeyboardInformationImpl.h
///
///            
///
/// @author    Download-Fritz
/// @date      13/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __KEYBOARD_INFORMATION_IMPL_H__
#define __KEYBOARD_INFORMATION_IMPL_H__

#include <Protocol/KeyboardInformation.h>

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
  );

// gKeyboardInfoIdVendor
extern UINT16 gKeyboardInfoIdVendor;

// gKeyboardInfoCountryCode
extern UINT8 gKeyboardInfoCountryCode;

// gKeyboardInfoIdProduct
extern UINT16 gKeyboardInfoIdProduct;

#endif // ifndef __KEYBOARD_INFORMATION_IMPL_H__


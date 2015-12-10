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
/// @file      Include/Driver/AppleSmcIo.h
///
///            
///
/// @author    Download-Fritz
/// @date      10/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_SMC_IO_DRV_H__
#define __APPLE_SMC_IO_DRV_H__

// AppleSmcIoMain
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
AppleSmcIoMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

#endif // ifndef __APPLE_SMC_IO_DRV_H__

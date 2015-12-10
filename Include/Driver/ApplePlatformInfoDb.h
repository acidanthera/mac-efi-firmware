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
/// @file      Include/Driver/ApplePlatformInfoDb.h
///
///            
///
/// @author    Download-Fritz
/// @date      11/10/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_PLATFORM_INFO_DB_DRV_H__
#define __APPLE_PLATFORM_INFO_DB_DRV_H__

// ApplePlatformInfoDBMain
///
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
EFI_STATUS
EFIAPI
ApplePlatformInfoDBMain (
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
	);

#endif // ifndef __APPLE_PLATFORM_INFO_DB_DRV_H__

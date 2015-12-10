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
/// @file      Include/Driver/EfiDevicePathPropertyDatabase.h
///
///            Apple's driver to manage Device Properties from firmware
///
/// @author    Download-Fritz
/// @date      23/02/2015: Initial version
/// @date      15/03/2015: Updated documentation
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __DEVICE_PATH_PROPERTY_DATABASE_DRV_H__
#define __DEVICE_PATH_PROPERTY_DATABASE_DRV_H__

// EfiDevicePathPropertyDatabaseMain
///
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
EFI_STATUS
EFIAPI
EfiDevicePathPropertyDatabaseMain (
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
	);

#endif // ifndef __DEVICE_PATH_PROPERTY_DATABASE_DRV_H__

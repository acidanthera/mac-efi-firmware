///
/// @file      Include/Driver/EfiDevicePathPropertyDatabase.h
///
///            Apple's driver to manage Device Properties from firmware
///
/// @author    Download-Fritz
/// @date      23/02/2015: Initial version
/// @date      15/03/2015: Updated documentation
/// @copyright The decompilation is of an educational purpose to better understand the behavior of the
///            Apple EFI implementation and making use of it. In no way is the content's usage licensed
///            or allowed. All rights remain at Apple Inc. To be used under the terms of 'Fair use'.
///

//
// CREDITS:
//   Reversed from EfiDevicePathPropertyDatabase.efi, which is Apple Inc. property
//   Decompiled by Download-Fritz
//

#ifndef _DEVICE_PATH_PROPERTY_DATABASE_DRV_H_
#define _DEVICE_PATH_PROPERTY_DATABASE_DRV_H_

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

#endif // ifndef _DEVICE_PATH_PROPERTY_DATABASE_DRV_H_

#ifndef __APPLE_PLATFORM_INFO_DB_H__
#define __APPLE_PLATFORM_INFO_DB_H__

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

#endif // ifndef __APPLE_PLATFORM_INFO_DB_H__
